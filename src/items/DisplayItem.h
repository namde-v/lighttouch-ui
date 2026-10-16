// DisplayItem.h — the QQuickItem that presents guest frames (surface 2).
//
// The display path bypasses the bridge (ABI.md §4 rule 5): LTDisplayAdapter
// (UI thread) owns the shm mapping and runs the FrameRing reader protocol on
// kLTEventFrameAvailable; this item only uploads the held segment's pixels to
// a GPU texture on the render thread in updatePaintNode. The writer never
// touches the held segment, so the render thread needs no atomics and makes
// no lt_ calls.

#pragma once

#include <QQuickItem>
#include <QSGSimpleTextureNode>

#include "items/FrameRing.h"

class LTDisplayAdapter;

class DisplayItem : public QQuickItem {
    Q_OBJECT
    // The adapter that owns the ring mapping and the reader protocol.
    Q_PROPERTY(LTDisplayAdapter *display READ display WRITE setDisplay NOTIFY displayChanged)
    // Texture filtering: nearest when zoom shows guest pixels >= backing
    // pixels (lt_zoom_draws_nearest), else linear.
    Q_PROPERTY(bool drawsNearest READ drawsNearest WRITE setDrawsNearest NOTIFY drawsNearestChanged)

public:
    explicit DisplayItem(QQuickItem *parent = nullptr);

    LTDisplayAdapter *display() const { return m_display; }
    void setDisplay(LTDisplayAdapter *d);

    bool drawsNearest() const { return m_drawsNearest; }
    void setDrawsNearest(bool n);

signals:
    void displayChanged();
    void drawsNearestChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;

private:
    LTDisplayAdapter *m_display = nullptr;
    bool m_drawsNearest = false;
};
