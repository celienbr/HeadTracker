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

class ExpoGuiTests : public QObject {
    Q_OBJECT
private slots:
    void dialogAcceptCancel() {
        TrackerSettings ts;
        ExpoDialog cancel(ts);
        cancel.show();
        cancel.findChild<QSpinBox*>("panExpo")->setValue(99);
        QTest::mouseClick(cancel.findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Cancel),Qt::LeftButton);
        QVERIFY(!cancel.applyTo(ts)); QCOMPARE(ts.getPanExpo(),uint8_t(0));
        ExpoDialog ok(ts); ok.show();
        auto *p=ok.findChild<QSpinBox*>("panExpo");
        auto *t=ok.findChild<QSpinBox*>("tiltExpo");
        auto *r=ok.findChild<QSpinBox*>("rollExpo");
        p->setValue(101); QCOMPARE(p->value(),100);
        t->setValue(-1); QCOMPARE(t->value(),0);
        p->setValue(51); t->setValue(36); r->setValue(100);
        QTest::mouseClick(ok.findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Ok),Qt::LeftButton);
        QVERIFY(ok.applyTo(ts)); QCOMPARE(ts.getPanExpo(),uint8_t(51));
        QCOMPARE(ts.getTltExpo(),uint8_t(36)); QCOMPARE(ts.getRllExpo(),uint8_t(100));
        ExpoDialog again(ts);
        QCOMPARE(again.findChild<QSpinBox*>("panExpo")->value(),51);
        QCOMPARE(again.findChild<QSpinBox*>("tiltExpo")->value(),36);
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
        QByteArray set=board.dataout(); QVERIFY(set.contains("\"Cmd\":\"Set\""));
        QVERIFY(set.contains("\"panexpo\":71")); QVERIFY(set.contains("\"tltexpo\":22"));
        QVERIFY(!set.contains("Flash"));
        QByteArray nak(1,char(0x15)); board.dataIn(nak); QCOMPARE(board.dataout(),set);
        QByteArray ack(1,char(0x06)); board.dataIn(ack);
        QByteArray flash=board.dataout(); QVERIFY(flash.contains("\"Cmd\":\"Flash\""));
        board.dataIn(nak); QCOMPARE(board.dataout(),flash);
        board.dataIn(ack); QVERIFY(board.dataout().isEmpty());
        // Device response, serialized as on the wire (framing/CRC bytes supplied).
        QByteArray payload="{\"Cmd\":\"Set\",\"panexpo\":12,\"tltexpo\":34,\"rllexpo\":56}";
        QByteArray response=QByteArray(1,char(2))+payload+QByteArray(2,char(0x7f))+QByteArray(1,char(3));
        board.dataIn(response); QCOMPARE(ts.getPanExpo(),uint8_t(12)); QCOMPARE(ts.getTltExpo(),uint8_t(34)); QCOMPARE(ts.getRllExpo(),uint8_t(56));
        board.disconnected();
    }
    void mainWindowAndWizard() {
        MainWindow window; window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
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
