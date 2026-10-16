// tst_surface7.cpp — Surface 7 (log window + console) tests, offscreen.
//
// Drives the log adapter + LogView/LogWindow/ConsoleSplit QML against the
// mock bridge:
//   - sources, read, follow → live lines, rotation resets,
//   - filter (case-insensitive, Xcode-style), pause, clear,
//   - console split: toggle, drag-resize, height persistence.

#include <QtTest>
#include <QSignalSpy>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickItem>

#include "Bridge.h"
#include "adapters/LTLogAdapter.h"
#include "adapters/LTSettingsAdapter.h"

namespace {
QString qmlPath(const char *rel) {
    return QString::fromUtf8(LT_QML_DIR) + QString::fromUtf8(rel);
}
} // namespace

class TstSurface7 : public QObject {
    Q_OBJECT

    struct Fixture {
        Bridge bridge;
        LTLogAdapter log;
        LTSettingsAdapter settings;
        QQmlEngine engine;
        Fixture() : log(&bridge), settings(&bridge) {
            QString err;
            QVERIFY2(bridge.initialize(&err), qPrintable(err));
            QQmlContext *ctx = engine.rootContext();
            ctx->setContextProperty(QStringLiteral("bridge"), &bridge);
            ctx->setContextProperty(QStringLiteral("log"), &log);
            ctx->setContextProperty(QStringLiteral("settings"), &settings);
            log.refreshSources();
        }
        QObject *load(const char *rel) {
            QQmlComponent comp(&engine, QUrl::fromLocalFile(qmlPath(rel)));
            if (comp.isError()) {
                qWarning() << comp.errors();
                return nullptr;
            }
            return comp.create(engine.rootContext());
        }
    };

private slots:
    void initTestCase() {
        qputenv("LTM_BRIDGE_MOCK", "1");
        qputenv("LTM_BRIDGE_MOCK_QUIET", "1");
        qunsetenv("LTM_APPCORE_LIB");
        QCoreApplication::setOrganizationName(QStringLiteral("LightTouchTest"));
        QCoreApplication::setApplicationName(QStringLiteral("tst_surface7"));
    }

    void sourcesList() {
        Fixture f;
        QVERIFY(f.log.sources().size() >= 1);
        QVERIFY(f.log.sources().contains(QStringLiteral("app")));
        QCOMPARE(f.log.currentSource(), QStringLiteral("app"));
    }

    void readAppendsLines() {
        Fixture f;
        QSignalSpy spy(&f.log, SIGNAL(linesChanged()));
        QVERIFY(spy.isValid());
        f.log.read(0);
        QVERIFY(spy.size() >= 1);
        QVERIFY(f.log.lines().size() >= 2);
        QVERIFY(f.log.lines()[0].contains(QStringLiteral("LightTouch")));
        QCOMPARE(f.log.offset(), quint64(58));
    }

    void followReceivesLiveLines() {
        Fixture f;
        f.log.setFollowing(true);
        QVERIFY(f.log.following());
        const int before = f.log.lines().size();
        f.bridge.mockFireLogLine(QStringLiteral("app"),
                                 QStringLiteral("[app] live line\n"),
                                 100);
        QTRY_COMPARE_WITH_TIMEOUT(f.log.lines().size(), before + 1, 5000);
        QCOMPARE(f.log.offset(), quint64(100));
        // A line from another source is ignored.
        f.bridge.mockFireLogLine(QStringLiteral("other"),
                                 QStringLiteral("[other] nope\n"), 200);
        QTest::qWait(100);
        QCOMPARE(f.log.lines().size(), before + 1);
        f.log.setFollowing(false);
    }

    void rotationResets() {
        Fixture f;
        f.log.read(0);
        QVERIFY(!f.log.lines().isEmpty());
        f.bridge.mockFireLogRotated(QStringLiteral("app"));
        QTRY_VERIFY_WITH_TIMEOUT(f.log.lines().isEmpty(), 5000);
        QCOMPARE(f.log.offset(), quint64(0));
    }

    void clearEmpties() {
        Fixture f;
        f.log.read(0);
        QVERIFY(!f.log.lines().isEmpty());
        // lt_log_clear is a command; the mock's rotated event does the reset.
        f.log.clear();
        f.bridge.mockFireLogRotated(QStringLiteral("app"));
        QTRY_VERIFY_WITH_TIMEOUT(f.log.lines().isEmpty(), 5000);
    }

    void logViewQmlFilter() {
        Fixture f;
        QObject *view = f.load("/views/LogView.qml");
        QVERIFY2(view != nullptr, "LogView failed to load");
        QVERIFY(view->findChild<QObject *>(QStringLiteral("logSourcePicker")) != nullptr);
        QVERIFY(view->findChild<QObject *>(QStringLiteral("logFilterField")) != nullptr);
        QVERIFY(view->findChild<QObject *>(QStringLiteral("logClearButton")) != nullptr);
        QVERIFY(view->findChild<QObject *>(QStringLiteral("logPauseBox")) != nullptr);
        // The Xcode-style filter: shownText() keeps only matching lines.
        f.log.read(0);
        view->setProperty("filter", QStringLiteral("mock core"));
        QVariant ret;
        QVERIFY(QMetaObject::invokeMethod(view, "shownText",
                                          Q_RETURN_ARG(QVariant, ret)));
        QVERIFY(ret.toString().contains(QStringLiteral("mock core")));
        QVERIFY(!ret.toString().contains(QStringLiteral("starting")));
        view->setProperty("filter", QString());
    }

    void consoleSplitToggleAndHeight() {
        Fixture f;
        f.settings.setConsoleHeight(-1); // isolate from previous runs
        QObject *split = f.load("/views/ConsoleSplit.qml");
        QVERIFY2(split != nullptr, "ConsoleSplit failed to load");
        QVERIFY(split->findChild<QObject *>(QStringLiteral("consoleBar")) != nullptr);
        QVERIFY(split->findChild<QObject *>(QStringLiteral("consoleToggle")) != nullptr);
        QVERIFY(split->findChild<QObject *>(QStringLiteral("consoleTextArea")) != nullptr);
        // Toggle collapses; the height persists across loads.
        QVERIFY(split->property("expanded").toBool());
        QVERIFY(QMetaObject::invokeMethod(split, "toggle"));
        QVERIFY(!split->property("expanded").toBool());
        QCOMPARE(f.settings.consoleHeight(), 0);
        QVERIFY(QMetaObject::invokeMethod(split, "toggle"));
        split->setProperty("consoleHeight", 250);
        QVERIFY(QMetaObject::invokeMethod(split, "commitHeight"));
        QCOMPARE(f.settings.consoleHeight(), 250);
    }

    void logWindowLoads() {
        Fixture f;
        QObject *win = f.load("/views/LogWindow.qml");
        QVERIFY2(win != nullptr, "LogWindow failed to load");
        QCOMPARE(win->property("title").toString(), QStringLiteral("Device Logs"));
    }
};

QTEST_MAIN(TstSurface7)
#include "tst_surface7.moc"
