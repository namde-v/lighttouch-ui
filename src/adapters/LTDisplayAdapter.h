// LTDisplayAdapter.h — the emulator display's UI-thread owner (surface 2).
//
// Owns the shm frame-ring mapping from kLTEventDisplayLinkReady and runs the
// FrameRing reader protocol on kLTEventFrameAvailable (UI thread only). The
// DisplayItem (render thread) uploads the held segment's pixels; the mapping
// is shared via shared_ptr so the UI thread can drop it without racing an
// in-flight upload. No lt_ calls happen on the render thread (ABI.md §4.4).
//
// Also fronts zoom (lt_display_set_zoom / lt_zoom_stops_json / lt_zoom_step,
// per-board memory in QSettings) and the guest input seams (lt_input_touch,
// lt_input_key/text, lt_input_tilt, lt_display_set_panel_size,
// lt_resize_drag). The Qt→HID key map is best-effort (ABI.md §6 gap 8).

#pragma once

#include <atomic>
#include <memory>

#include <QMutex>

#include "LTAdapterBase.h"
#include "items/FrameRing.h"

// One mapped ring segment; munmaps on last release.
struct RingMapping {
    uchar *base = nullptr;
    size_t size = 0;
    RingMapping(uchar *b, size_t s) : base(b), size(s) {}
};

class LTDisplayAdapter : public LTAdapterBase {
    Q_OBJECT
    Q_PROPERTY(QString deviceUuid READ deviceUuid WRITE setDeviceUuid NOTIFY deviceUuidChanged)
    Q_PROPERTY(bool linkReady READ linkReady NOTIFY linkChanged)
    Q_PROPERTY(int frameWidth READ frameWidth NOTIFY linkChanged)
    Q_PROPERTY(int frameHeight READ frameHeight NOTIFY linkChanged)
    Q_PROPERTY(int rotationDegrees READ rotationDegrees NOTIFY rotationChanged)
    // Effective points per guest pixel (from kLTEventZoomChanged).
    Q_PROPERTY(double zoomPoints READ zoomPoints NOTIFY zoomChanged)
    Q_PROPERTY(QString zoomModeName READ zoomModeName NOTIFY zoomChanged)
    Q_PROPERTY(QString zoomMode READ zoomMode WRITE setZoomMode NOTIFY zoomChanged)

public:
    explicit LTDisplayAdapter(Bridge *bridge, QObject *parent = nullptr);
    ~LTDisplayAdapter() override;

    QString deviceUuid() const { return m_uuid; }
    void setDeviceUuid(const QString &uuid);

    bool linkReady() const { return m_mapping != nullptr; }
    int frameWidth() const { return m_frameW; }
    int frameHeight() const { return m_frameH; }
    int rotationDegrees() const { return m_rotation; }
    double zoomPoints() const { return m_zoomPoints; }
    QString zoomModeName() const { return m_zoomModeName; }
    QString zoomMode() const { return m_zoomMode; }
    void setZoomMode(const QString &mode);

    // Render-thread entry: the held frame's pixels, or null. The returned
    // shared_ptr keeps the mapping alive for the upload.
    std::shared_ptr<RingMapping> mappingForRender() const;
    int heldFrameIndex() const { return m_heldIndex.load(); }
    int frameStride() const { return m_stride; }

    // Called by the DisplayItem on the UI thread.
    void requestUpdate();

    Q_INVOKABLE void refresh();
    // Zoom ladder (Mac: View menu + ⌘+/−; zoom remembered per board).
    Q_INVOKABLE void zoomIn();
    Q_INVOKABLE void zoomOut();
    Q_INVOKABLE void setZoomModeExplicit(const QString &mode) { setZoomMode(mode); }
    // This pane's zoom context (fit/physical/backing points); QML updates it
    // on resize so the ⌘+/− ladder stays correct.
    Q_INVOKABLE void setZoomContext(double fit, double physical, double backing);
    // ZoomContext.drawsNearest: nearest when zoom shows guest pixels >=
    // backing pixels, else linear.
    Q_INVOKABLE bool drawsNearest(double backing);
    // Guest input seams (normalized 0..1, y-down; phases kLTTouch*).
    Q_INVOKABLE void sendTouch(int phase, double x, double y, int touchId);
    Q_INVOKABLE void sendKey(int qtKey, int down, int modifiers);
    Q_INVOKABLE void typeText(const QString &text);
    Q_INVOKABLE void sendTilt(double angle, double pitch);
    // Free-form: apply a panel size (restarts the device when running).
    Q_INVOKABLE void setPanelSize(int width, int height);
    Q_INVOKABLE QString resizeDrag(const QString &board, double startW, double startH,
                                   double edgeX, double edgeY, double deltaX, double deltaY,
                                   double pointsPerPixel, bool symmetric, bool quarterTurned);

signals:
    void deviceUuidChanged();
    void linkChanged();
    void rotationChanged();
    void zoomChanged();
    void frameAvailable();

protected:
    void onBridgeEvent(int kind, quint64 device, quint64 seq,
                       const QVariantMap &payload) override;

private:
    void openLink(int fd, int w, int h, int stride, quint64 generation);
    void closeLink();
    void acquireFrame();
    void zoomStep(int direction);

    QString m_uuid;
    std::shared_ptr<RingMapping> m_mapping; // guarded by m_mapMutex
    mutable QMutex m_mapMutex;
    ltframe::Reader m_reader;
    std::atomic<int> m_heldIndex{-1};
    int m_frameW = 0;
    int m_frameH = 0;
    int m_stride = 0;
    quint64 m_generation = 0;
    int m_rotation = 0;
    double m_zoomPoints = 1.0;
    double m_fitPoints = 1.0;
    double m_physicalPoints = -1.0;
    double m_backingScale = 1.0;
    QString m_zoomModeName;
    QString m_zoomMode = QStringLiteral("fit");
};
