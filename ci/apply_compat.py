#!/usr/bin/env python3
"""Small, checked patches against the pinned v2.40 source tree."""
from pathlib import Path
import subprocess
import sys

root = Path(sys.argv[1])
assets = Path(__file__).resolve().parent

def patch(path, old, new):
    p = root / path
    text = p.read_text(encoding="utf-8")
    if text.count(old) != 1:
        raise RuntimeError(f"{path}: expected exactly one match for {old[:80]!r}")
    p.write_text(text.replace(old, new, 1), encoding="utf-8")

for source, target in [("expo_dialog.h", "gui/src/expo_dialog.h"),
                       ("flash_settings.h", "firmware/src/src/include/flash_settings.h")]:
    (root / target).write_bytes((assets / source).read_bytes())

# Keep generated settings reproducible, and reject invalid JSON before narrowing.
patch("settings/buildfwsettings.py",
      'for row in s.settings:\n  f.write("    v = json[',
      '''for row in s.settings:
  if row[s.colname] in ("PanExpo", "TltExpo", "RllExpo"):
    name = row[s.colname]
    f.write('    v = json["' + name.lower() + '"]; if(v.is<int>() && v.as<int>() >= 0 && v.as<int>() <= 100) {set' + name + '(v.as<int>());}' + "\\n")
    continue
  f.write("    v = json[''')

patch("gui/src/trackersettings.h", "    void clear() {_data.clear(); _setting.clear(); _features.clear();}",
      "    bool supportsExpo() const { return expoSupported; }\n"
      "    void clear() {_data.clear(); _setting.clear(); _deviceSettings.clear(); _features.clear(); expoSupported = false;}")
patch("gui/src/trackersettings.h", "    QStringList bleAddresses;",
      "    bool expoSupported = false;\n    QStringList bleAddresses;")
patch("gui/src/trackersettings.cpp", '#include "trackersettings.h"',
      '''#include "trackersettings.h"
#include <cmath>

static bool isExpoKey(const QString &key) {
    return key == "panexpo" || key == "tltexpo" || key == "rllexpo";
}
static bool validExpo(const QVariant &value) {
    bool ok = false;
    const double x = value.toDouble(&ok);
    return ok && std::isfinite(x) && x >= 0 && x <= 100 && std::floor(x) == x;
}''')
patch("gui/src/trackersettings.cpp", "    QStringList keys = settings->allKeys();",
      '''    // Old INI files have linear curves, regardless of the current device values.
    for (const QString &key : {QString("panexpo"), QString("tltexpo"), QString("rllexpo")})
        _setting[key] = 0;
    QStringList keys = settings->allKeys();''')
patch("gui/src/trackersettings.cpp", "        _setting[key] = settings->value(key);",
      '''        if (isExpoKey(key) && !validExpo(settings->value(key))) continue;
        _setting[key] = settings->value(key);''')
patch("gui/src/trackersettings.cpp", "    QStringList keys = data.keys();",
      '''    expoSupported = data.contains("panexpo") && data.contains("tltexpo") && data.contains("rllexpo");
    for (const QString &key : {QString("panexpo"), QString("tltexpo"), QString("rllexpo")}) {
        if (!data.contains(key) || !validExpo(data.value(key))) {
            _setting[key] = 0;
            _deviceSettings.remove(key);
            expoSupported = false;
        }
    }
    QStringList keys = data.keys();''')
patch("gui/src/trackersettings.cpp", "        _setting[key] = data.value(key);",
      "        if (isExpoKey(key) && !validExpo(data.value(key))) continue;\n        _setting[key] = data.value(key);")
patch("gui/src/trackersettings.cpp", "        // Current Data != Data on Device",
      "        if (isExpoKey(key) && !expoSupported) continue;\n        // Current Data != Data on Device")

# Flush the debounce before Flash. NAK must retry the in-flight packet, not the last queued packet.
patch("gui/src/mainwindow.cpp", "void MainWindow::storeToNVM()\n{",
      "void MainWindow::storeToNVM()\n{\n    saveToRAMTimer.stop();")
patch("gui/src/boardjson.cpp", '    sendSerialJSON("Flash");',
      '    saveToRAM(); // Queue all pending changes before the Flash request.\n    sendSerialJSON("Flash");')
patch("gui/src/boardjson.cpp", "            serialDataOut += jsonqueue.dequeue();",
      "            lastjson = jsonqueue.dequeue();\n            serialDataOut += lastjson;")
patch("gui/src/boardjson.cpp", "    lastjson = (char)0x02 + json.toLatin1()",
      "    const QByteArray packet = (char)0x02 + json.toLatin1()")
patch("gui/src/boardjson.cpp", "        jsonqueue.enqueue(lastjson);", "        jsonqueue.enqueue(packet);")
patch("gui/src/boardjson.cpp", "    // Add serial data to the TX buffer, emit a signal it's ready",
      "    lastjson = packet;\n    // Add serial data to the TX buffer, emit a signal it's ready")

# v2.40 declared an ACK timeout but never connected one. Bound lost-ACK retries.
patch("gui/src/boardjson.h", "    QTimer updatesettingstmr;", "    QTimer updatesettingstmr;\n    QTimer ackTimer;")
patch("gui/src/boardjson.cpp", "    trkset = ts;", """    trkset = ts;
    ackTimer.setSingleShot(true);
    ackTimer.setInterval(ACKNAK_TIMEOUT);
    connect(&ackTimer, &QTimer::timeout, this, &BoardJson::nakError);""")
patch("gui/src/boardjson.cpp", "void BoardJson::dataIn(QByteArray &data)\n{",
      "void BoardJson::dataIn(QByteArray &data)\n{\n    if (data.isEmpty()) return;")
patch("gui/src/boardjson.cpp", "        QByteArray crcs = data.mid(data.length()-3,2);",
      "        if (data.size() < 5) return;\n        QByteArray crcs = data.mid(data.length()-3,2);")
patch("gui/src/boardjson.cpp", "        //uint16_t crc = crcs[0] << 8 | crcs[1];",
      "        uint16_t crc = uint8_t(crcs[0]) | (uint16_t(uint8_t(crcs[1])) << 8);")
patch("gui/src/boardjson.cpp", "        //uint crc2 = escapeCRC(uCRC16Lib::calculate(stripped.data(), stripped.length()));\n        /*if(crc != crc2) {\n            qDebug() << \"CRC Fault\" << crc << crc2;\n        } else {*/\n            parseIncomingJSON(QJsonDocument::fromJson(stripped).object().toVariantMap());\n        //}",
      """        uint16_t crc2 = escapeCRC(uCRC16Lib::calculate(stripped.data(), stripped.length()));
        if (crc != crc2) {
            emit addToLog("ERROR: Incoming JSON CRC mismatch; ignored\\n");
            return;
        }
        parseIncomingJSON(QJsonDocument::fromJson(stripped).object().toVariantMap());""")
patch("gui/src/boardjson.cpp", "        // Clear the fault counter\n        jsonwaitingack = 0;",
      """        if (jsonwaitingack == 0) return; // Ignore unsolicited/duplicate ACK.
        ackTimer.stop();
        const QString acknowledged = QJsonDocument::fromJson(lastjson.mid(1,lastjson.size()-6)).object()["Cmd"].toString();
        bool queuedSet = false;
        for (const auto &pending : jsonqueue)
            if (QJsonDocument::fromJson(pending.mid(1,pending.size()-6)).object()["Cmd"].toString() == "Set") queuedSet = true;
        if (acknowledged == "Set") {
            savedToRAM = !queuedSet;
            emit paramSendComplete();
        }
        // This ACK confirms receipt only; physical flash must be checked after reboot.
        if (acknowledged == "Flash") savedToNVM = !queuedSet;
        jsonwaitingack = 0;""")
patch("gui/src/boardjson.cpp", "            jsonwaitingack = 1;\n            emit serialTxReady();",
      "            jsonwaitingack = 1;\n            ackTimer.start();\n            emit serialTxReady();")
patch("gui/src/boardjson.cpp", "    savedToRAM = true;\n    savedToNVM = false;",
      "    savedToRAM = false;\n    savedToNVM = false;")
patch("gui/src/boardjson.cpp", '    sendSerialJSON("Flash");\n    savedToNVM = true;',
      '    savedToNVM = false;\n    sendSerialJSON("Flash");')
patch("gui/src/boardjson.cpp", "void BoardJson::startData()\n{", "void BoardJson::startData()\n{\n    ackTimer.stop();")
patch("gui/src/boardjson.cpp", "void BoardJson::disconnected()\n{",
      "void BoardJson::disconnected()\n{\n    ackTimer.stop();\n    serialDataOut.clear();")
patch("gui/src/boardjson.cpp", "    jsonwaitingack = 1; // Set as faulted until ACK returned",
      "    jsonwaitingack = 1; // Set as faulted until ACK returned\n    ackTimer.start();")
patch("gui/src/boardjson.cpp", "void BoardJson::nakError()\n{",
      "void BoardJson::nakError()\n{\n    if (jsonwaitingack == 0) return;\n    ackTimer.stop();")
patch("gui/src/boardjson.cpp", "        // Pause a bit, give time for device to catch up\n        Sleep(TX_FAULT_PAUSE);",
      "        // Nonblocking, bounded retry of the outstanding packet.")
patch("gui/src/boardjson.cpp", "        jsonwaitingack++;", "        jsonwaitingack++;\n        ackTimer.start();")

# Recenter-on-tilt is an input gesture; Expo must not change its threshold.
patch("firmware/src/src/sense.cpp", "    // Reset on tilt",
      '''    const uint16_t rollout_for_reset = expoChannel(rollout, trkset.getRll_Cnt(),
        trkset.getRll_Min(), trkset.getRll_Max(), 0);
    // Reset on tilt''')
patch("firmware/src/src/sense.cpp", "if (rollout_ui == trkset.getRll_Max())", "if (rollout_for_reset == trkset.getRll_Max())")
patch("firmware/src/src/sense.cpp", "if (rollout_ui == trkset.getRll_Min())", "if (rollout_for_reset == trkset.getRll_Min())")

patch("firmware/src/src/trackersettings.cpp", '#include "soc_flash.h"',
      '#include "soc_flash.h"\n#include "flash_settings.h"')
patch("firmware/src/src/trackersettings.cpp", "  int len = serializeJson(json, buffer, TX_RNGBUF_SIZE);",
      "  const size_t len = serializeFlashSettings(json, buffer, sizeof(buffer));")
patch("firmware/src/src/trackersettings.cpp", "  if (socWriteFlash(buffer, len)) {",
      '''  if (len == 0) {
    LOG_ERR("Settings exceed flash serialization buffer; not written");
    return;
  }
  if (socWriteFlash(buffer, len)) {''')
patch("firmware/src/src/trackersettings.cpp", 'LOG_ERR("Flash Write Failed size(%d)", len);',
      'LOG_ERR("Flash Write Failed size(%u)", static_cast<unsigned>(len));')
patch("firmware/src/src/trackersettings.cpp", 'LOG_INF("Saved to Flash size(%d)", len);',
      'LOG_INF("Saved to Flash size(%u)", static_cast<unsigned>(len));')

revision = subprocess.check_output(["git", "rev-parse", "--short=12", "HEAD"], cwd=assets, text=True).strip()
patch("gui/src/mainwindow.cpp", 'setWindowTitle(windowTitle() + " " + guiDisplayVersion);',
      'setWindowTitle(windowTitle() + " " + guiDisplayVersion + " Expo ' + revision + '");')

# Downloaded files must be writable after installation in Program Files.
patch("gui/src/firmwarewizard.h", "#include <QSettings>", "#include <QSettings>\n#include <QStandardPaths>")
patch("gui/src/firmwarewizard.h",
      '#elif defined(WINDOWS)\nconstexpr char bossac_programmer[] = "bossac.exe";\nconst QString localfirmlist = "firmware.ini";\nconst QString localfirmware = "online.fmw";',
      '#elif defined(WINDOWS)\nconstexpr char bossac_programmer[] = "bossac.exe";\nconst QString localfirmlist = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/HeadTrackerExpo-firmware.ini";\nconst QString localfirmware = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/HeadTrackerExpo-online.fmw";')
