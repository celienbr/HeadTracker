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
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLabel>
#include "expo_curve_preview.h"

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
        // A single click opens exactly the same yellow PopupSlider as the red center.
        auto *lowPopup=bar.findChild<PopupSlider*>("deadLowPopup");
        auto *highPopup=bar.findChild<PopupSlider*>("deadHighPopup");
        QVERIFY(lowPopup); QVERIFY(highPopup);
        bar.repaint();
        const QPoint amberLow=bar.deadLowHandleRect().center().toPoint();
        qInfo() << "Deadzone low" << bar.deadLowValue() << "click" << amberLow
                << "hitbox" << bar.deadLowHandleRect();
        QVERIFY(bar.deadLowHandleRect().contains(amberLow));
        QTest::mouseClick(&bar,Qt::LeftButton,Qt::NoModifier,amberLow);
        QVERIFY(lowPopup->isVisible());
        auto *lowField=lowPopup->findChild<QDoubleSpinBox*>();
        QVERIFY(lowField);
        lowField->setValue(51);
        QCOMPARE(bar.deadLowValue(),51);
        QCOMPARE(bar.deadHighValue()>0,true);
        QCOMPARE(lowSpy.count(),2);
        bar.repaint();
        const QPoint amberHigh=bar.deadHighHandleRect().center().toPoint();
        QVERIFY(bar.deadHighHandleRect().contains(amberHigh));
        QTest::mouseClick(&bar,Qt::LeftButton,Qt::NoModifier,amberHigh);
        QVERIFY(highPopup->isVisible());
        auto *highField=highPopup->findChild<QDoubleSpinBox*>();
        QVERIFY(highField);
        highField->setValue(82);
        QCOMPARE(bar.deadHighValue(),82);
        QCOMPARE(bar.deadLowValue(),51);
        QCOMPARE(highSpy.count(),2);
        QVERIFY(bar.grab().save("../../../evidence/deadzone-sliders.png"));
        bar.close();
    }
    void curvePreviewFollowsSelectionAndExpo() {
        TrackerSettings model;
        model.setAllData(model.allData());
        model.setPanExpo(38); model.setTltExpo(23); model.setRllExpo(72);
        model.setPan_Min(1020); model.setPan_Max(1940); model.setPan_Cnt(1510);
        model.setPanDZLow(40); model.setPanDZHigh(13);
        model.setTltDZLow(5); model.setTltDZHigh(25);
        model.setRllDZLow(9); model.setRllDZHigh(63);
        ExpoDialog dialog(model);
        dialog.show();
        QVERIFY(QTest::qWaitForWindowExposed(&dialog));
        auto *plot=dialog.findChild<ExpoCurvePreview*>("expoCurvePreview");
        auto *selector=dialog.findChild<QComboBox*>("curveAxis");
        auto *title=dialog.findChild<QLabel*>("curveGraphTitle");
        auto *expo=dialog.findChild<QSpinBox*>("expoPercent");
        auto *left=dialog.findChild<QSpinBox*>("deadzoneLeft");
        auto *right=dialog.findChild<QSpinBox*>("deadzoneRight");
        auto *center=dialog.findChild<QSpinBox*>("curveCenter");
        QVERIFY(plot); QVERIFY(selector); QVERIFY(title);
        QVERIFY(expo); QVERIFY(left); QVERIFY(right); QVERIFY(center);
        QCOMPARE(selector->count(),3);
        QCOMPARE(plot->currentExpo(),38);
        QCOMPARE(plot->currentDeadLeft(),uint16_t(40));
        QCOMPARE(plot->currentDeadRight(),uint16_t(13));
        QCOMPARE(plot->outputFor(-15),uint16_t(1510));
        QCOMPARE(plot->outputFor(11),uint16_t(1510));
        QVERIFY(plot->outputFor(90)>1510);
        QVERIFY(title->text().contains("Pan"));
        expo->setValue(80);
        QCOMPARE(plot->currentExpo(),80);
        left->setValue(48);
        QCOMPARE(plot->currentDeadLeft(),uint16_t(48));
        center->setValue(1520);
        QCOMPARE(plot->outputFor(-20),uint16_t(1520));
        selector->setCurrentIndex(2);
        QCOMPARE(plot->currentExpo(),72);
        QCOMPARE(plot->currentDeadLeft(),uint16_t(9));
        QCOMPARE(plot->currentDeadRight(),uint16_t(63));
        QVERIFY(plot->grab().save("../../../evidence/deadzone-curve-preview.png"));
        QVERIFY(dialog.grab().save("../../../evidence/english-expo-dialog.png"));
        selector->setCurrentIndex(1);
        QCOMPARE(plot->currentExpo(),23);
        QCOMPARE(plot->currentDeadRight(),uint16_t(25));
        // Axis switching retains other drafts; nothing reaches device on Cancel.
        selector->setCurrentIndex(0);
        QCOMPARE(plot->currentExpo(),80);
        QCOMPARE(plot->currentDeadLeft(),uint16_t(48));
        QCOMPARE(model.getPanExpo(),uint8_t(38));
        dialog.reject();
        QVERIFY(!dialog.applyTo(model));
        QCOMPARE(model.getPanExpo(),uint8_t(38));
        QCOMPARE(model.getPan_Cnt(),uint16_t(1510));
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
