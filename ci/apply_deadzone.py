#!/usr/bin/env python3
"""Apply extra center deadzone support after applying the known-good Expo patches.

Upstream source is always pinned v2.40; never modify a running aircraft firmware.
"""
from pathlib import Path
import sys

root = Path(sys.argv[1])
assets = Path(__file__).resolve().parent

def patch(path, old, new):
    p = root / path
    s = p.read_text(encoding="utf-8")
    if s.count(old) != 1:
        raise RuntimeError(f"{path}: expected one match, found {s.count(old)}: {old[:80]!r}")
    p.write_text(s.replace(old, new, 1), encoding="utf-8")
    print("Deadzone patch:", path)

names = ("PanDZLow", "PanDZHigh", "TltDZLow", "TltDZHigh", "RllDZLow", "RllDZHigh")
jsonkeys = tuple(n.lower() for n in names)
rows = "".join(f"u16,Setting,{n},0,0,250,{n} microseconds,,,," + "\n" for n in names)

patch("settings/settings.csv",
      "u8,Setting,RllExpo,0,0,100,Roll Expo (percent),,,,\n",
      "u8,Setting,RllExpo,0,0,100,Roll Expo (percent),,,,\n" + rows)

patch("settings/buildfwsettings.py",
      'if row[s.colname] in ("PanExpo", "TltExpo", "RllExpo"):',
      'if row[s.colname] in ("PanExpo", "TltExpo", "RllExpo", ' +
      ', '.join(repr(n) for n in names) + '):')
patch("settings/buildfwsettings.py",
      '    f.write(\'    v = json["\' + name.lower() + \'"]; if(v.is<int>() && v.as<int>() >= 0 && v.as<int>() <= 100) {set\' + name + \'(v.as<int>());}\' + "\\n")',
      '    maximum = "250" if "DZ" in name else "100"\n' +
      '    f.write(\'    v = json["\' + name.lower() + \'"]; if(v.is<int>() && v.as<int>() >= 0 && v.as<int>() <= \' + maximum + \') {set\' + name + \'(v.as<int>());}\' + "\\n")')

# Firmware output: keep exactly the Expo path when all deadzones == 0.
source = root / "firmware/src/src/expo_deadzone.h"
source.write_text((assets / "expo_deadzone.h").read_text(encoding="utf-8"), encoding="utf-8")
patch("firmware/src/src/sense.cpp", '#include "expo_curve.h"\n',
      '#include "expo_curve.h"\n#include "expo_deadzone.h"\n')
for axis, name in (("tilt", "Tlt"), ("roll", "Rll"), ("pan", "Pan")):
    old = f"""    const uint16_t {axis}out_ui = expoChannel(
        {axis}out, trkset.get{name}_Cnt(), trkset.get{name}_Min(), trkset.get{name}_Max(),
        trkset.get{name}Expo());"""
    new = f"""    const uint16_t {axis}out_ui = expoDeadzoneChannel(
        {axis}out, trkset.get{name}_Cnt(), trkset.get{name}_Min(), trkset.get{name}_Max(),
        trkset.get{name}Expo(), trkset.get{name}DZLow(), trkset.get{name}DZHigh());"""
    patch("firmware/src/src/sense.cpp", old, new)

# Firmware <-> GUI capability negotiation. A previous Expo-only firmware must
# not receive unknown deadzone settings. Old saved INI files mean zero DZ.
keys_q = ", ".join(f'QString("{key}")' for key in jsonkeys)
patch("gui/src/trackersettings.h",
      '    bool supportsExpo() const { return expoSupported; }',
      '    bool supportsExpo() const { return expoSupported; }\n'
      '    bool supportsDeadzone() const { return deadzoneSupported; }')
patch("gui/src/trackersettings.h",
      '    bool expoSupported = false;',
      '    bool expoSupported = false;\n    bool deadzoneSupported = false;')
patch("gui/src/trackersettings.h",
      '_features.clear(); expoSupported = false;',
      '_features.clear(); expoSupported = false; deadzoneSupported = false;')
patch("gui/src/trackersettings.cpp",
      'static bool isExpoKey(const QString &key) {',
      'static bool isDeadzoneKey(const QString &key) {\n'
      f'    return {" || ".join(f"key == {chr(34)}{key}{chr(34)}" for key in jsonkeys)};\n'
      '}\n'
      'static bool validDeadzone(const QVariant &value) {\n'
      '    bool ok = false;\n'
      '    const double v = value.toDouble(&ok);\n'
      '    return ok && std::isfinite(v) && v >= 0 && v <= 250 && std::floor(v) == v;\n'
      '}\n'
      'static bool isExpoKey(const QString &key) {')
patch("gui/src/trackersettings.cpp",
      '    // Old INI files have linear curves, regardless of the current device values.',
      '    // Old INI files have no zone morte: start both sides at zero.\n'
      f'    for (const QString &key : {{{keys_q}}}) _setting[key] = 0;\n'
      '    // Old INI files have linear curves, regardless of the current device values.')
patch("gui/src/trackersettings.cpp",
      '        if (isExpoKey(key) && !validExpo(settings->value(key))) continue;',
      '        if (isExpoKey(key) && !validExpo(settings->value(key))) continue;\n'
      '        if (isDeadzoneKey(key) && !validDeadzone(settings->value(key))) continue;')
patch("gui/src/trackersettings.cpp",
      '    expoSupported = data.contains("panexpo") && data.contains("tltexpo") && data.contains("rllexpo");',
      '    deadzoneSupported = true;\n'
      f'    for (const QString &key : {{{keys_q}}}) {{\n'
      '        if (!data.contains(key) || !validDeadzone(data.value(key))) {\n'
      '            _setting[key] = 0;\n'
      '            _deviceSettings.remove(key);\n'
      '            deadzoneSupported = false;\n'
      '        }\n'
      '    }\n'
      '    expoSupported = data.contains("panexpo") && data.contains("tltexpo") && data.contains("rllexpo");')
patch("gui/src/trackersettings.cpp",
      '        if (isExpoKey(key) && !validExpo(data.value(key))) continue;',
      '        if (isExpoKey(key) && !validExpo(data.value(key))) continue;\n'
      '        if (isDeadzoneKey(key) && !validDeadzone(data.value(key))) continue;')
patch("gui/src/trackersettings.cpp",
      '        if (isExpoKey(key) && !expoSupported) continue;',
      '        if (isExpoKey(key) && !expoSupported) continue;\n'
      '        if (isDeadzoneKey(key) && !deadzoneSupported) continue;')

# Directly add the six thumbs to the existing 3 interactive min/max bars.
h = "gui/src/servominmax.h"
patch(h, '#include <QTimer>\n', '#include <QTimer>\n#include <QtGlobal>\n')
patch(h, '    int maxTravel() {return max_travel;}\n',
      '    int maxTravel() {return max_travel;}\n'
      '    int deadLowValue() const { return deadLow; }\n'
      '    int deadHighValue() const { return deadHigh; }\n')
patch(h, '    void setActualPosition(int value) {actValue = value;update();}\n',
      '    void setActualPosition(int value) {actValue = value;update();}\n'
      '    void setDeadLow(int value) { deadLow = qBound(0, value, 250); update(); }\n'
      '    void setDeadHigh(int value) { deadHigh = qBound(0, value, 250); update(); }\n'
      '    void setDeadzoneEnabled(bool enabled) { deadzoneEnabled = enabled; update(); }\n')
patch(h, '    void centerChanged(int value);\n',
      '    void centerChanged(int value);\n'
      '    void deadLowChanged(int value);\n'
      '    void deadHighChanged(int value);\n')
patch(h, '    int actValue;\n',
      '    int actValue;\n'
      '    int deadLow = 0, deadHigh = 0;\n'
      '    int deadLowStart = 0, deadHighStart = 0;\n'
      '    bool deadLowSelected = false, deadHighSelected = false;\n'
      '    bool deadzoneEnabled = false;\n'
      '    QRectF deadLowSlider, deadHighSlider;\n'
      '    int deadLowLimit() const { return qMax(0, qMin(250, c_value - min_val - 1)); }\n'
      '    int deadHighLimit() const { return qMax(0, qMin(250, max_val - c_value - 1)); }\n')

cpp = "gui/src/servominmax.cpp"
patch(cpp, '#include <QToolTip>\n',
      '#include <QToolTip>\n#include <QInputDialog>\n#include <QtMath>\n')
patch(cpp, '    centerSlider.translate(-sliderw/2,0);\n',
      '    centerSlider.translate(-sliderw/2,0);\n\n'
      '    // Independent upper/lower handles remain distinguishable at 0 us,\n'
      '    // when both occupy the red center line (upper = low, lower = high).\n'
      '    const double scale = (width()-padding) / travel;\n'
      '    deadLowSlider = QRectF(centerSlider.center().x() - qMin(deadLow, deadLowLimit())*scale - 5,\n'
      '                           height()*0.06, 10, height()*0.33);\n'
      '    deadHighSlider = QRectF(centerSlider.center().x() + qMin(deadHigh, deadHighLimit())*scale - 5,\n'
      '                            height()*0.61, 10, height()*0.33);\n')
patch(cpp, '    // Draw Left Slider\n',
      '    // Gold translucent band indicates the zero-output region.\n'
      '    if (deadzoneEnabled) {\n'
      '        painter.fillRect(QRectF(deadLowSlider.center().x(), linetop,\n'
      '                   qMax(0.0, deadHighSlider.center().x()-deadLowSlider.center().x()),\n'
      '                   linebot-linetop), QColor(255, 179, 0, 95));\n'
      '    }\n'
      '    // Draw Left Slider\n')
patch(cpp, '    // Draw the current output\n',
      '    // The two independent amber thumbs are on different vertical halves.\n'
      '    if (deadzoneEnabled) {\n'
      '        painter.setPen(QPen(QColor(92,64,0), 1));\n'
      '        painter.setBrush(QColor(255,179,0));\n'
      '        painter.drawRoundedRect(deadLowSlider, 2, 2);\n'
      '        painter.drawRoundedRect(deadHighSlider, 2, 2);\n'
      '    }\n\n'
      '    // Draw the current output\n')
patch(cpp, '    minSliderSelected = false;\n    maxSliderSelected = false;\n    centerSliderSelected = false;\n}\n\nvoid ServoMinMax::mousePressEvent',
      '    if (deadLowSelected && deadLow != deadLowStart) emit deadLowChanged(deadLow);\n'
      '    if (deadHighSelected && deadHigh != deadHighStart) emit deadHighChanged(deadHigh);\n'
      '    minSliderSelected = false;\n    maxSliderSelected = false;\n'
      '    centerSliderSelected = false;\n'
      '    deadLowSelected = deadHighSelected = false;\n}\n\nvoid ServoMinMax::mousePressEvent')
patch(cpp, '    centerSliderSelected = false;\n\n    cntspinbox->hide();',
      '    centerSliderSelected = false;\n'
      '    deadLowSelected = deadHighSelected = false;\n\n'
      '    cntspinbox->hide();')
patch(cpp, '        if(minSlider.contains(event->pos())) {',
      '        if (deadzoneEnabled && deadLowSlider.contains(event->pos())) {\n'
      '            mouseDownPoint = event->globalPosition();\n'
      '            deadLowSelected = true;\n'
      '            deadLowStart = deadLow;\n'
      '        } else if (deadzoneEnabled && deadHighSlider.contains(event->pos())) {\n'
      '            mouseDownPoint = event->globalPosition();\n'
      '            deadHighSelected = true;\n'
      '            deadHighStart = deadHigh;\n'
      '        } else if(minSlider.contains(event->pos())) {')
patch(cpp, '    if(minSliderSelected) {\n        min_val = min_start - xtr;',
      '    if (deadLowSelected) {\n'
      '        deadLow = qBound(0, deadLowStart + qRound(xtr), deadLowLimit());\n'
      '        QToolTip::showText(event->globalPosition().toPoint(),\n'
      '            tr("Zone morte inferieure : %1 us").arg(deadLow), this);\n'
      '        update();\n'
      '    } else if (deadHighSelected) {\n'
      '        deadHigh = qBound(0, deadHighStart - qRound(xtr), deadHighLimit());\n'
      '        QToolTip::showText(event->globalPosition().toPoint(),\n'
      '            tr("Zone morte superieure : %1 us").arg(deadHigh), this);\n'
      '        update();\n'
      '    } else if(minSliderSelected) {\n        min_val = min_start - xtr;')
patch(cpp, 'void ServoMinMax::mouseDoubleClickEvent(QMouseEvent *event)\n{\n',
      'void ServoMinMax::mouseDoubleClickEvent(QMouseEvent *event)\n{\n'
      '    if (deadzoneEnabled && event->button() == Qt::LeftButton) {\n'
      '        const bool low = deadLowSlider.contains(event->pos());\n'
      '        const bool high = deadHighSlider.contains(event->pos());\n'
      '        if (low || high) {\n'
      '            bool ok = false;\n'
      '            int oldValue = low ? deadLow : deadHigh;\n'
      '            int v = QInputDialog::getInt(this, tr("Zone morte"),\n'
      '                low ? tr("Cote inferieur (us)") : tr("Cote superieur (us)"),\n'
      '                oldValue, 0, low ? deadLowLimit() : deadHighLimit(), 1, &ok);\n'
      '            if (ok && v != oldValue) {\n'
      '                if (low) { setDeadLow(v); emit deadLowChanged(v); }\n'
      '                else { setDeadHigh(v); emit deadHighChanged(v); }\n'
      '            }\n'
      '            event->accept();\n'
      '            return;\n'
      '        }\n'
      '    }\n')
patch(cpp, '            if(minSlider.contains(helpEvent->pos())) {',
      '            if (deadzoneEnabled && deadLowSlider.contains(helpEvent->pos())) {\n'
      '                QToolTip::showText(glbpoint, tr("Zone morte inferieure : %1 us").arg(deadLow));\n'
      '            } else if (deadzoneEnabled && deadHighSlider.contains(helpEvent->pos())) {\n'
      '                QToolTip::showText(glbpoint, tr("Zone morte superieure : %1 us").arg(deadHigh));\n'
      '            } else if(minSlider.contains(helpEvent->pos())) {')

# Bind graphical thumbs to live UI, running same debounce as center/endstop controls.
ui = "gui/src/mainwindow.cpp"
for widget, axis in (("Pan", "Pan"), ("Tilt", "Tlt"), ("Roll", "Rll")):
    old = f'    connect(ui->servo{widget}, &ServoMinMax::centerChanged, this, &MainWindow::updateFromUI);'
    new = old + f'\n    connect(ui->servo{widget}, &ServoMinMax::deadLowChanged, this, &MainWindow::updateFromUI);' \
              + f'\n    connect(ui->servo{widget}, &ServoMinMax::deadHighChanged, this, &MainWindow::updateFromUI);'
    patch(ui, old, new)
    old = f'    ui->servo{widget}->setMinimum(trkset.get{axis}_Min());'
    new = old + f'\n    ui->servo{widget}->setDeadzoneEnabled(trkset.supportsDeadzone());' \
              + f'\n    ui->servo{widget}->setDeadLow(trkset.get{axis}DZLow());' \
              + f'\n    ui->servo{widget}->setDeadHigh(trkset.get{axis}DZHigh());'
    patch(ui, old, new)
    old = f'    trkset.set{axis}_Max(ui->servo{widget}->maximumValue());'
    new = old + f'\n    if (trkset.supportsDeadzone()) {{' \
              + f'\n        trkset.set{axis}DZLow(ui->servo{widget}->deadLowValue());' \
              + f'\n        trkset.set{axis}DZHigh(ui->servo{widget}->deadHighValue());' \
              + '\n    }'
    patch(ui, old, new)

# Distinguish this experimental branch in the titlebar.
patch(ui, '    // Serial Connection\n',
      '    setWindowTitle(windowTitle() + tr(" + Deadzone (experimental)"));\n'
      '    // Serial Connection\n')
