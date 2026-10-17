#include <QtTest>
#include <QAction>
#include <QTemporaryDir>
#include <QPushButton>
#include <QJsonDocument>
#include <QJsonObject>
#include "expo_dialog.h"
#include "boardjson.h"
#include "mainwindow.h"
#include "firmwarewizard.h"
#include "ucrc16lib.h"
#include "servominmax.h"

class DeadzoneGuiTests : public QObject {
    Q_OBJECT
private slots:
    void independentBarsAndSettings() {
        TrackerSettings model;
        auto withDeadzone = model.allData();
        model.setAllData(withDeadzone);
        QVERIFY(model.supportsDeadzone());
        model.setPanDZLow(11); model.setPanDZHigh(43);
        model.setTltDZLow(6); model.setTltDZHigh(29);
        model.setRllDZLow(1); model.setRllDZHigh(77);
        QTemporaryDir tmp; QVERIFY(tmp.isValid());
        QSettings prefs(tmp.filePath("new.ini"),QSettings::IniFormat);
        model.storeSettings(&prefs); prefs.sync();
        TrackerSettings restored; restored.loadSettings(&prefs);
        QCOMPARE(restored.getPanDZLow(),uint16_t(11));
        QCOMPARE(restored.getPanDZHigh(),uint16_t(43));
        QCOMPARE(restored.getTltDZLow(),uint16_t(6));
        QCOMPARE(restored.getTltDZHigh(),uint16_t(29));
        QCOMPARE(restored.getRllDZLow(),uint16_t(1));
        QCOMPARE(restored.getRllDZHigh(),uint16_t(77));

        QSettings old(tmp.filePath("expo-only.ini"),QSettings::IniFormat);
        old.setValue("panexpo",55);
        restored.loadSettings(&old);
        QCOMPARE(restored.getPanDZLow(),uint16_t(0));
        QCOMPARE(restored.getPanDZHigh(),uint16_t(0));
        QCOMPARE(restored.getRllDZLow(),uint16_t(0));
        old.setValue("pandzlow", 65536);
        old.setValue("pandzhigh", -1);
        restored.loadSettings(&old);
        QCOMPARE(restored.getPanDZLow(),uint16_t(0));
        QCOMPARE(restored.getPanDZHigh(),uint16_t(0));

        withDeadzone.remove("pandzlow");
        model.setAllData(withDeadzone);
        QVERIFY(!model.supportsDeadzone());
        model.setPanDZHigh(40);
        QVERIFY(!model.changedData().contains("pandzhigh"));
        model.clear();
        QVERIFY(!model.supportsDeadzone());

        ServoMinMax bar;
        bar.resize(420,48);
        bar.setMinimum(1050);
        bar.setMaximum(1950);
        bar.setCenter(1500);
        bar.setDeadzoneEnabled(true);
        bar.setDeadLow(0); bar.setDeadHigh(0);
        bar.show();
        QVERIFY(QTest::qWaitForWindowExposed(&bar));
        QSignalSpy lowSpy(&bar,&ServoMinMax::deadLowChanged);
        QSignalSpy highSpy(&bar,&ServoMinMax::deadHighChanged);
        const int x = qRound(5. + (1500.-988.)/(2012.-988.)*(bar.width()-10.));
        // At zero both handles share the center x but are separated vertically.
        const QPoint lowStart(x, qRound(bar.height()*0.25));
        const QPoint highStart(x, qRound(bar.height()*0.75));
        QTest::mousePress(&bar,Qt::LeftButton,Qt::NoModifier,lowStart);
        QPoint newLow(x-24,lowStart.y());
        QMouseEvent lowMove(QEvent::MouseMove,QPointF(newLow),QPointF(bar.mapToGlobal(newLow)),
                            Qt::NoButton,Qt::LeftButton,Qt::NoModifier);
        QCoreApplication::sendEvent(&bar,&lowMove);
        QTest::mouseRelease(&bar,Qt::LeftButton,Qt::NoModifier,newLow);
        QVERIFY(bar.deadLowValue()>0);
        QCOMPARE(bar.deadHighValue(),0);
        QCOMPARE(lowSpy.count(),1);
        QCOMPARE(highSpy.count(),0);
        QTest::mousePress(&bar,Qt::LeftButton,Qt::NoModifier,highStart);
        QPoint newHigh(x+28,highStart.y());
        QMouseEvent highMove(QEvent::MouseMove,QPointF(newHigh),QPointF(bar.mapToGlobal(newHigh)),
                             Qt::NoButton,Qt::LeftButton,Qt::NoModifier);
        QCoreApplication::sendEvent(&bar,&highMove);
        QTest::mouseRelease(&bar,Qt::LeftButton,Qt::NoModifier,newHigh);
        QVERIFY(bar.deadHighValue()>0);
        QCOMPARE(highSpy.count(),1);
        QVERIFY(bar.grab().save("../../../evidence/deadzone-sliders.png"));
        bar.close();
    }
    void protocolWithDeadzone() {
        TrackerSettings ts;
        ts.setAllData(ts.allData());
        ts.setDataMatched();
        BoardJson board(&ts);
        ts.setPanDZLow(13); ts.setPanDZHigh(62);
        ts.setTltDZLow(40); ts.setTltDZHigh(12);
        ts.setRllDZLow(7); ts.setRllDZHigh(18);
        board.saveToNVM();
        QByteArray set=board.dataout();
        QVERIFY(set.contains("\"pandzlow\":13"));
        QVERIFY(set.contains("\"pandzhigh\":62"));
        QVERIFY(set.contains("\"tltdzlow\":40"));
        QVERIFY(set.contains("\"tltdzhigh\":12"));
        QVERIFY(set.contains("\"rlldzlow\":7"));
        QVERIFY(set.contains("\"rlldzhigh\":18"));
        QByteArray ack(1,char(0x06));
        board.dataIn(ack);
        QByteArray flash=board.dataout();
        QVERIFY(flash.contains("\"Cmd\":\"Flash\""));
        board.dataIn(ack);
        QVERIFY(board.isBoardSavedToNVM());
        board.disconnected();
    }
};

QTEST_MAIN(DeadzoneGuiTests)
#include "deadzone_gui_tests.moc"
