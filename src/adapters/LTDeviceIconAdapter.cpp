// LTDeviceIconAdapter.cpp — board → bundled artwork.

#include "LTDeviceIconAdapter.h"

LTDeviceIconAdapter::LTDeviceIconAdapter(QObject *parent) : QObject(parent) {
    // The art resources compile into the ltbridge static lib; static libs
    // do not auto-register their resources, so initialize them here (the
    // documented Q_INIT_RESOURCE pattern). Every binary that constructs
    // this adapter (app + tests) gets qrc:/art/*.png. PNG is used instead
    // of SVG: the QtSvg image plugin is not shipped on all targets.
    Q_INIT_RESOURCE(art);
}

QString LTDeviceIconAdapter::iconSource(const QString &board) const {
    const QString b = board.toLower();
    // iPhone boards (m68/n82/n88/n90/n92/n94/n41/n42/n48/n49/…): any board
    // string containing a known iPhone family marker. The iPod touch 2nd gen
    // (n72ap) is checked first so it does not fall into the iPhone bucket.
    if (b.contains(QStringLiteral("n72")))
        return QStringLiteral("qrc:/art/ipod.png");
    if (b.startsWith(QStringLiteral("k48")) || b.startsWith(QStringLiteral("k93"))
        || b.startsWith(QStringLiteral("k94")) || b.startsWith(QStringLiteral("k95"))
        || b.startsWith(QStringLiteral("j1")) || b.startsWith(QStringLiteral("j2"))
        || b.contains(QStringLiteral("ipad")))
        return QStringLiteral("qrc:/art/ipad.png");
    if (b.startsWith(QStringLiteral("n")) || b.startsWith(QStringLiteral("m"))
        || b.contains(QStringLiteral("iphone")) || b.contains(QStringLiteral("ipod")))
        return QStringLiteral("qrc:/art/iphone.png");
    return QStringLiteral("qrc:/art/device-generic.png");
}
