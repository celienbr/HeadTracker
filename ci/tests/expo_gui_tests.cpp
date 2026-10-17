#include <QtTest>
#include <QAction>
#include <QComboBox>
#include <QTemporaryDir>
#include <QPushButton>
#include <QJsonDocument>
#include <QJsonObject>
#include "expo_dialog.h"
#include "boardjson.h"
#include "mainwindow.h"
#include "firmwarewizard.h"
#include "ucrc16lib.h"

class ExpoGuiTests : public QObject {
    Q_OBJECT
private slots:
    void dialogAcceptCancel() {
        TrackerSettings ts;
        ts.setAllData(ts.allData());
        QVERIFY(ts.supportsExpo());
        QVERIFY(ts.supportsDeadzone());
        const uint8_t initial=ts.getPanExpo();
        ExpoDialog cancel(ts);
        cancel.show();
        auto *c=cancel.findChild<QSpinBox*>("expoPercent");
        auto *selectCancel=cancel.findChild<QComboBox*>("curveAxis");
        QVERIFY(c); QVERIFY(selectCancel);
        c->setValue(99);
        QTest::mouseClick(cancel.findChild<QDialogButtonBox*>()->button(
            QDialogButtonBox::Cancel),Qt::LeftButton);
        QVERIFY(!cancel.applyTo(ts));
        QCOMPARE(ts.getPanExpo(),initial);

        ExpoDialog ok(ts);
        ok.show();
        auto *axis=ok.findChild<QComboBox*>("curveAxis");
        auto *expo=ok.findChild<QSpinBox*>("expoPercent");
        auto *min=ok.findChild<QSpinBox*>("curveMin");
        auto *max=ok.findChild<QSpinBox*>("curveMax");
        auto *center=ok.findChild<QSpinBox*>("curveCenter");
        auto *left=ok.findChild<QSpinBox*>("deadzoneLeft");
        auto *right=ok.findChild<QSpinBox*>("deadzoneRight");
        QVERIFY(axis);QVERIFY(expo);QVERIFY(min);QVERIFY(max);
        QVERIFY(center);QVERIFY(left);QVERIFY(right);
        QCOMPARE(axis->count(),3);
        expo->setValue(101);QCOMPARE(expo->value(),100);
        expo->setValue(51);
        min->setValue(1060);max->setValue(1930);center->setValue(1510);
        left->setValue(38);right->setValue(22);
        axis->setCurrentIndex(1);expo->setValue(36);
        axis->setCurrentIndex(2);expo->setValue(100);
        axis->setCurrentIndex(0);
        QCOMPARE(expo->value(),51);
        QCOMPARE(min->value(),1060);
        QCOMPARE(max->value(),1930);
        QCOMPARE(center->value(),1510);
        QCOMPARE(left->value(),38);
        QCOMPARE(right->value(),22);
        // No modifications before explicit OK.
        QCOMPARE(ts.getPanExpo(),initial);
        QTest::mouseClick(ok.findChild<QDialogButtonBox*>()->button(
            QDialogButtonBox::Ok),Qt::LeftButton);
        QVERIFY(ok.applyTo(ts));
        QCOMPARE(ts.getPanExpo(),uint8_t(51));
        QCOMPARE(ts.getTltExpo(),uint8_t(36));
        QCOMPARE(ts.getRllExpo(),uint8_t(100));
        QCOMPARE(ts.getPan_Min(),uint16_t(1060));
        QCOMPARE(ts.getPan_Max(),uint16_t(1930));
        QCOMPARE(ts.getPan_Cnt(),uint16_t(1510));
        QCOMPARE(ts.getPanDZLow(),uint16_t(38));
        QCOMPARE(ts.getPanDZHigh(),uint16_t(22));
        ExpoDialog again(ts);
        QCOMPARE(again.findChild<QSpinBox*>("expoPercent")->value(),51);
    }
    void iniAndCompatibility() {
        QTemporaryDir dir; QVERIFY(dir.isValid());
        TrackerSettings ts; auto all=ts.allData(); ts.setAllData(all); QVERIFY(ts.supportsExpo());
        ts.setPanExpo(10); ts.setTltExpo(20); ts.setRllExpo(30); ts.setPan_Gain(7);
        QSettings settings(dir.filePath("new.ini"),QSettings::IniFormat);
        ts.storeSettings(&settings); settings.sync();
        TrackerSettings loaded; loaded.loadSettings(&settings);
        QCOMPARE(loaded.getPanExpo(),uint8_t(10)); QCOMPARE(loaded.getTltExpo(),uint8_t(20));
        QCOMPARE(loaded.getRllExpo(),uint8_t(30)); QCOMPARE(loaded.getPan_Gain(),7.f);
        QSettings old(dir.filePath("old.ini"),QSettings::IniFormat); old.setValue("pan_gain",8);
        loaded.loadSettings(&old); QCOMPARE(loaded.getPanExpo(),uint8_t(0)); QCOMPARE(loaded.getTltExpo(),uint8_t(0));
        old.setValue("panexpo",256); old.setValue("tltexpo",35.5); old.setValue("rllexpo",-1);
        loaded.loadSettings(&old); QCOMPARE(loaded.getPanExpo(),uint8_t(0)); QCOMPARE(loaded.getTltExpo(),uint8_t(0)); QCOMPARE(loaded.getRllExpo(),uint8_t(0));
        all.remove("panexpo"); all.remove("tltexpo"); all.remove("rllexpo");
        ts.setAllData(all); QVERIFY(!ts.supportsExpo());
        ts.setPanExpo(50); QVERIFY(!ts.changedData().contains("panexpo"));
        ts.clear(); QVERIFY(!ts.supportsExpo());
    }
    void protocolSaveAndRetry() {
        TrackerSettings ts; ts.setAllData(ts.allData()); ts.setDataMatched();
        BoardJson board(&ts);
        ts.setPanExpo(71); ts.setTltExpo(22); ts.setRllExpo(13);
        // This is the same method invoked by the Save Settings button.
        board.saveToNVM();
        QVERIFY(!board.isBoardSavedToRAM()); QVERIFY(!board.isBoardSavedToNVM());
        QByteArray set=board.dataout(); QVERIFY(set.contains("\"Cmd\":\"Set\""));
        QVERIFY(set.contains("\"panexpo\":71")); QVERIFY(set.contains("\"tltexpo\":22"));
        QVERIFY(!set.contains("Flash"));
        QByteArray nak(1,char(0x15)); board.dataIn(nak); QCOMPARE(board.dataout(),set);
        QByteArray ack(1,char(0x06)); board.dataIn(ack);
        QByteArray flash=board.dataout(); QVERIFY(flash.contains("\"Cmd\":\"Flash\""));
        QVERIFY(board.isBoardSavedToRAM()); QVERIFY(!board.isBoardSavedToNVM());
        board.dataIn(nak); QCOMPARE(board.dataout(),flash);
        board.dataIn(ack); QVERIFY(board.dataout().isEmpty());
        QVERIFY(board.isBoardSavedToNVM());
        // Device response, serialized as on the wire (framing/CRC bytes supplied).
        QByteArray payload="{\"Cmd\":\"Set\",\"panexpo\":12,\"tltexpo\":34,\"rllexpo\":56}";
        uint16_t crc=BoardJson::escapeCRC(uCRC16Lib::calculate(payload.data(),payload.size()));
        QByteArray response=QByteArray(1,char(2))+payload+QByteArray(1,char(crc&255))+QByteArray(1,char(crc>>8))+QByteArray(1,char(3));
        board.dataIn(response); QCOMPARE(ts.getPanExpo(),uint8_t(12)); QCOMPARE(ts.getTltExpo(),uint8_t(34)); QCOMPARE(ts.getRllExpo(),uint8_t(56));
        response[response.size()-3] = char(response[response.size()-3]^0x11);
        ts.setPanExpo(99); board.dataIn(response); QCOMPARE(ts.getPanExpo(),uint8_t(99));
        board.disconnected();
    }
    void lostAckIsBounded() {
        TrackerSettings ts; ts.setAllData(ts.allData()); ts.setDataMatched();
        BoardJson board(&ts); QSignalSpy failure(&board,&BoardJson::paramSendFailure);
        ts.setPanExpo(65); board.saveToRAM(); auto packet=board.dataout();
        QTest::qWait(600); QCOMPARE(board.dataout(),packet);
        QTRY_COMPARE_WITH_TIMEOUT(failure.count(),1,6000);
        QVERIFY(!board.isBoardSavedToRAM());
        board.disconnected(); QVERIFY(board.dataout().isEmpty());
    }
    void mainWindowAndWizard() {
        MainWindow window; window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(window.grab().save("../../../evidence/main-window.png"));
        QVERIFY(window.findChild<QPushButton*>("cmdConnect"));
        QVERIFY(window.findChild<QPushButton*>("cmdSaveNVM"));
        auto *action=window.findChild<QAction*>("expoAction"); QVERIFY(action);
        QTimer::singleShot(100,[](){
            auto *message=qobject_cast<QMessageBox*>(QApplication::activeModalWidget());
            QVERIFY(message); message->accept();
        });
        action->trigger(); // Offline guard, with no serial device.
        FirmwareWizard wizard; wizard.show();
        QVERIFY(wizard.findChild<QPushButton*>("cmdOpenFile"));
        QVERIFY(wizard.findChild<QPushButton*>("cmdProgram"));
        QVERIFY(!wizard.findChild<QPushButton*>("cmdProgram")->isEnabled());
        wizard.close(); window.close();
    }
};
QTEST_MAIN(ExpoGuiTests)
#include "expo_gui_tests.moc"
