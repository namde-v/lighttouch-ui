// DisplayItem.cpp — the QQuickItem that presents guest frames (surface 2).

#include "items/DisplayItem.h"

#include "adapters/LTDisplayAdapter.h"

#include <QSGSimpleTextureNode>
#include <QQuickWindow>

DisplayItem::DisplayItem(QQuickItem *parent)
    : QQuickItem(parent) {
    setFlag(ItemHasContents, true);
}

void DisplayItem::setDisplay(LTDisplayAdapter *d) {
    if (d == m_display)
        return;
    if (m_display)
        disconnect(m_display, nullptr, this, nullptr);
    m_display = d;
    if (m_display) {
        // A new frame was acquired on the UI thread: repaint on the render
        // thread. The held segment is stable across this handoff by the ring
        // protocol (the writer never touches it).
        connect(m_display, &LTDisplayAdapter::frameAvailable,
                this, [this]() { update(); });
        connect(m_display, &LTDisplayAdapter::linkChanged,
                this, [this]() { update(); });
    }
    emit displayChanged();
    update();
}

void DisplayItem::setDrawsNearest(bool n) {
    if (n == m_drawsNearest)
        return;
    m_drawsNearest = n;
    emit drawsNearestChanged();
    update();
}

QSGNode *DisplayItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) {
    auto *node = static_cast<QSGSimpleTextureNode *>(oldNode);
    if (!m_display) {
        delete node;
        return nullptr;
    }
    const std::shared_ptr<RingMapping> mapping = m_display->mappingForRender();
    const int held = m_display->heldFrameIndex();
    const int w = m_display->frameWidth();
    const int h = m_display->frameHeight();
    const int stride = m_display->frameStride();
    if (!mapping || !mapping->base || held < 0 || w <= 0 || h <= 0) {
        delete node;
        return nullptr;
    }
    // BGRA bytes on little-endian == QImage::Format_ARGB32. The QImage wraps
    // the held segment without copying; createTextureFromImage uploads.
    const uchar *bits = mapping->base + ltframe::kStatusBytes
        + static_cast<size_t>(held) * static_cast<size_t>(h) * stride;
    const QImage img(bits, w, h, stride, QImage::Format_ARGB32);
    QSGTexture *texture = window()->createTextureFromImage(img);
    texture->setFiltering(m_drawsNearest ? QSGTexture::Nearest : QSGTexture::Linear);
    if (!node)
        node = new QSGSimpleTextureNode();
    node->setTexture(texture);
    node->setOwnsTexture(true);
    node->setRect(boundingRect());
    // Filtering per ZoomContext.drawsNearest is applied above; the rect
    // follows the item, which QML sizes at frameSize * zoomPoints.
    return node;
}
