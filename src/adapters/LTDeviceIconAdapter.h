// LTDeviceIconAdapter.h — per-board device artwork (surface 1).
//
// Mirrors UI/Board+Icon.swift: the Mac reads Finder's per-product-type
// artwork via AppleDeviceType; on Linux/Windows there is no such source, so
// the Qt side bundles flat per-form-factor silhouettes as Qt resources.
// Marketing names ("iPod touch", …) are owned by the core's row JSON — this
// adapter only resolves the board string to an artwork URL, with a generic
// fallback for boards the port does not declare.

#pragma once

#include <QObject>
#include <QString>

class LTDeviceIconAdapter : public QObject {
    Q_OBJECT
public:
    explicit LTDeviceIconAdapter(QObject *parent = nullptr);

    // Board string (e.g. "n88ap") → "qrc:/art/iphone.svg" etc.
    // Never empty: unknown boards get the generic silhouette.
    Q_INVOKABLE QString iconSource(const QString &board) const;
};
