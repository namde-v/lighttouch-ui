// LTDisplayAdapter.cpp — the emulator display's UI-thread owner (surface 2).

#include "LTDisplayAdapter.h"

#include <QDebug>
#include <QSettings>

#ifdef Q_OS_WIN
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

namespace {

// --- shm mapping ---------------------------------------------------------
// The ABI's DisplayLinkReady carries one fd (POSIX) or one named-mapping
// handle value (Windows). No Qt abstraction attaches to a raw fd/handle,
// so this one helper is platform-branched; everything above it is portable.

struct MappedShm {
    uchar *base = nullptr;
    size_t size = 0;
};

MappedShm mapShm(int fd, size_t size) {
#ifdef Q_OS_WIN
    // The ABI's fd carries the low 32 bits of the mapping HANDLE value;
    // go through uint32_t to avoid sign-extension.
    HANDLE h = reinterpret_cast<HANDLE>(
        static_cast<quintptr>(static_cast<uint32_t>(fd)));
    // Read/write: the reader protocol publishes its hold in the status
    // segment (the writer never touches that slot).
    void *base = MapViewOfFile(h, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, size);
    return { static_cast<uchar *>(base), base ? size : 0 };
#else
    // PROT_WRITE: the reader protocol publishes its hold in the status
    // segment (the writer never touches that slot).
    void *base = ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    ::close(fd);
    if (base == MAP_FAILED)
        return { nullptr, 0 };
    return { static_cast<uchar *>(base), size };
#endif
}

void unmapShm(uchar *base, size_t size) {
    if (!base)
        return;
#ifdef Q_OS_WIN
    UnmapViewOfFile(base);
#else
    ::munmap(base, size);
#endif
}

} // namespace

// RingMapping releases the mapping via the helper above.
struct RingMappingDeleter {
    void operator()(RingMapping *m) const {
        unmapShm(m->base, m->size);
        delete m;
    }
};

LTDisplayAdapter::LTDisplayAdapter(Bridge *bridge, QObject *parent)
    : LTAdapterBase(bridge, parent) {}

LTDisplayAdapter::~LTDisplayAdapter() {
    closeLink();
}

void LTDisplayAdapter::setDeviceUuid(const QString &uuid) {
    if (uuid == m_uuid)
        return;
    closeLink();
    m_uuid = uuid;
    m_rotation = 0;
    // Restore the board's remembered zoom (ZoomMode.save, Qt-side).
    QSettings s;
    m_zoomMode = s.value(QStringLiteral("zoom/") + uuid,
                         QStringLiteral("fit")).toString();
    emit deviceUuidChanged();
    emit rotationChanged();
    emit zoomChanged();
    refresh();
}

void LTDisplayAdapter::setZoomMode(const QString &mode) {
    if (!ready() || m_uuid.isEmpty())
        return;
    const lt_device_t d = deviceHandle(m_uuid);
    if (d == kLTInvalidDevice)
        return;
    if (check(bridge()->api().lt_display_set_zoom(d, mode.toUtf8().constData()),
              "lt_display_set_zoom")) {
        m_zoomMode = mode;
        QSettings s;
        s.setValue(QStringLiteral("zoom/") + m_uuid, mode);
        emit zoomChanged();
    }
    // The effective points arrive via kLTEventZoomChanged.
}

void LTDisplayAdapter::zoomIn() { zoomStep(1); }
void LTDisplayAdapter::zoomOut() { zoomStep(-1); }

void LTDisplayAdapter::zoomStep(int direction) {
    if (!ready() || m_uuid.isEmpty())
        return;
    // The ladder needs this pane's fit/physical/backing numbers; QML supplies
    // them via setZoomContext before calling.
    const char *raw = nullptr;
    if (!check(bridge()->api().lt_zoom_stops_json(m_fitPoints, m_physicalPoints,
                                                  m_backingScale, &raw),
               "lt_zoom_stops_json"))
        return;
    const QString stopsJson = bridge()->takeString(raw);
    Q_UNUSED(stopsJson);
    const double next = bridge()->api().lt_zoom_step(
        m_fitPoints, m_physicalPoints, m_backingScale, m_zoomPoints, direction);
    if (next > 0)
        setZoomMode(QStringLiteral("p:") + QString::number(next));
}

void LTDisplayAdapter::setZoomContext(double fit, double physical, double backing) {
    m_fitPoints = fit;
    m_physicalPoints = physical;
    m_backingScale = backing;
}

bool LTDisplayAdapter::drawsNearest(double backing) {
    if (!ready())
        return false;
    return bridge()->api().lt_zoom_draws_nearest(m_zoomPoints, backing) != 0;
}

std::shared_ptr<RingMapping> LTDisplayAdapter::mappingForRender() const {
    QMutexLocker lock(&m_mapMutex);
    return m_mapping;
}

void LTDisplayAdapter::requestUpdate() {
    emit frameAvailable();
}

void LTDisplayAdapter::refresh() {
    if (!ready() || m_uuid.isEmpty())
        return;
    const lt_device_t d = deviceHandle(m_uuid);
    if (d == kLTInvalidDevice)
        return;
    const char *raw = nullptr;
    if (check(bridge()->api().lt_display_get_zoom(d, &raw), "lt_display_get_zoom")) {
        const QVariantMap m = jsonObject(bridge()->takeString(raw), "lt_display_get_zoom");
        const double points = m.value(QStringLiteral("points")).toDouble();
        const QString shown = m.value(QStringLiteral("shown")).toString();
        if (points > 0 && (points != m_zoomPoints || shown != m_zoomModeName)) {
            m_zoomPoints = points;
            m_zoomModeName = shown;
            emit zoomChanged();
        }
    }
}

void LTDisplayAdapter::openLink(int fd, int w, int h, int stride, quint64 generation) {
    closeLink();
    if (fd < 0 || w <= 0 || h <= 0 || stride < w * 4)
        return;
    const size_t size = ltframe::kStatusBytes + 3 * static_cast<size_t>(h) * stride;
    const MappedShm shm = mapShm(fd, size);
    if (!shm.base) {
        qWarning() << "LTDisplayAdapter: failed to map frame ring";
        return;
    }
    auto *raw = new RingMapping(shm.base, shm.size);
    std::shared_ptr<RingMapping> mapping(raw, RingMappingDeleter());
    m_reader.attach(reinterpret_cast<volatile uint64_t *>(shm.base), generation);
    m_frameW = w;
    m_frameH = h;
    m_stride = stride;
    m_generation = generation;
    m_heldIndex.store(-1);
    {
        QMutexLocker lock(&m_mapMutex);
        m_mapping = mapping;
    }
    emit linkChanged();
}

void LTDisplayAdapter::closeLink() {
    m_reader.detach();
    m_heldIndex.store(-1);
    {
        QMutexLocker lock(&m_mapMutex);
        m_mapping.reset();
    }
    m_frameW = m_frameH = m_stride = 0;
    emit linkChanged();
}

void LTDisplayAdapter::acquireFrame() {
    if (!m_reader.attached())
        return;
    const int idx = m_reader.acquire();
    if (idx != m_heldIndex.load()) {
        m_heldIndex.store(idx);
        emit frameAvailable();
    } else if (idx >= 0) {
        // Same frame re-published: still refresh (cheap; keeps paint alive).
        emit frameAvailable();
    }
}

void LTDisplayAdapter::onBridgeEvent(int kind, quint64 device, quint64 seq,
                                     const QVariantMap &payload) {
    Q_UNUSED(seq);
    if (!eventForMe(device))
        return;
    switch (kind) {
    case kLTEventDisplayLinkReady: {
        // Payload keys mirror LTEventDisplayLinkReady (eventPayloadToMap).
        openLink(payload.value(QStringLiteral("fd")).toInt(),
                 payload.value(QStringLiteral("width")).toInt(),
                 payload.value(QStringLiteral("height")).toInt(),
                 payload.value(QStringLiteral("stride")).toInt(),
                 payload.value(QStringLiteral("ringGeneration")).toULongLong());
        break;
    }
    case kLTEventDisplayLinkClosed:
        closeLink();
        break;
    case kLTEventFrameAvailable:
        acquireFrame();
        break;
    case kLTEventRotationChanged:
        m_rotation = payload.value(QStringLiteral("degrees")).toInt();
        emit rotationChanged();
        break;
    case kLTEventZoomChanged:
        m_zoomPoints = payload.value(QStringLiteral("points")).toDouble();
        m_zoomModeName = payload.value(QStringLiteral("mode")).toString();
        emit zoomChanged();
        break;
    default:
        break;
    }
}

// --- Guest input seams ----------------------------------------------------

void LTDisplayAdapter::sendTouch(int phase, double x, double y, int touchId) {
    if (!ready() || m_uuid.isEmpty())
        return;
    const lt_device_t d = deviceHandle(m_uuid);
    if (d == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_input_touch(d, phase, x, y, touchId), "lt_input_touch");
}

void LTDisplayAdapter::sendKey(int qtKey, int down, int modifiers) {
    if (!ready() || m_uuid.isEmpty())
        return;
    const lt_device_t d = deviceHandle(m_uuid);
    if (d == kLTInvalidDevice)
        return;
    // Best-effort Qt::Key → USB HID usage (ABI.md §6 gap 8: no authoritative
    // table exists; letters/digits/arrows/whitespace only).
    uint16_t hid = 0;
    if (qtKey >= Qt::Key_A && qtKey <= Qt::Key_Z)
        hid = 0x04 + (qtKey - Qt::Key_A);
    else if (qtKey >= Qt::Key_0 && qtKey <= Qt::Key_9)
        hid = (qtKey == Qt::Key_0) ? 0x27 : 0x1E + (qtKey - Qt::Key_1);
    else {
        switch (qtKey) {
        case Qt::Key_Return: case Qt::Key_Enter: hid = 0x28; break;
        case Qt::Key_Escape: hid = 0x29; break;
        case Qt::Key_Backspace: hid = 0x2A; break;
        case Qt::Key_Tab: hid = 0x2B; break;
        case Qt::Key_Space: hid = 0x2C; break;
        case Qt::Key_Left: hid = 0x50; break;
        case Qt::Key_Right: hid = 0x4F; break;
        case Qt::Key_Up: hid = 0x52; break;
        case Qt::Key_Down: hid = 0x51; break;
        default: break;
        }
    }
    if (hid == 0)
        return;
    int mods = 0;
    if (modifiers & Qt::ShiftModifier) mods |= kLTKeyModifierShift;
    if (modifiers & Qt::ControlModifier) mods |= kLTKeyModifierControl;
    if (modifiers & Qt::AltModifier) mods |= kLTKeyModifierAlt;
    if (modifiers & Qt::MetaModifier) mods |= kLTKeyModifierCommand;
    check(bridge()->api().lt_input_key(d, hid, down, mods), "lt_input_key");
}

void LTDisplayAdapter::typeText(const QString &text) {
    if (!ready() || m_uuid.isEmpty() || text.isEmpty())
        return;
    const lt_device_t d = deviceHandle(m_uuid);
    if (d == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_input_text(d, text.toUtf8().constData(), 0),
          "lt_input_text");
}

void LTDisplayAdapter::sendTilt(double angle, double pitch) {
    if (!ready() || m_uuid.isEmpty())
        return;
    const lt_device_t d = deviceHandle(m_uuid);
    if (d == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_input_tilt(d, angle, pitch), "lt_input_tilt");
}

void LTDisplayAdapter::setPanelSize(int width, int height) {
    if (!ready() || m_uuid.isEmpty())
        return;
    const lt_device_t d = deviceHandle(m_uuid);
    if (d == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_display_set_panel_size(d, width, height),
          "lt_display_set_panel_size");
}

QString LTDisplayAdapter::resizeDrag(const QString &board, double startW, double startH,
                                    double edgeX, double edgeY, double deltaX, double deltaY,
                                    double pointsPerPixel, bool symmetric, bool quarterTurned) {
    if (!ready())
        return {};
    const char *raw = nullptr;
    const LTResult r = bridge()->api().lt_resize_drag(
        board.toUtf8().constData(), startW, startH, edgeX, edgeY, deltaX, deltaY,
        pointsPerPixel, symmetric ? 1 : 0, quarterTurned ? 1 : 0, &raw);
    const QString json = bridge()->takeString(raw);
    if (!check(r, "lt_resize_drag"))
        return {};
    return json;
}
