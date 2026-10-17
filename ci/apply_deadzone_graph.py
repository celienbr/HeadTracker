#!/usr/bin/env python3
"""Append-only UI improvements on top of tested HeadTracker Expo Deadzone patch."""
from pathlib import Path
import sys

root = Path(sys.argv[1])
assets = Path(__file__).resolve().parent

def patch(path, old, new):
    f=root/path
    s=f.read_text(encoding="utf-8")
    if s.count(old)!=1:
        raise RuntimeError(f"{path}: expected 1 match, got {s.count(old)} for {old[:90]!r}")
    f.write_text(s.replace(old,new,1),encoding="utf-8")
    print("Updated graph/popup:",path)

# Reuse the EXACT firmware curve code in the Qt preview (no approximation).
for name in ("expo_curve.h","expo_deadzone.h","expo_curve_preview.h","expo_dialog.h"):
    target=root/"gui/src"/name
    target.write_bytes((assets/name).read_bytes())

# Existing amber deadzone handles use the same PopupSlider as the RED center.
h="gui/src/servominmax.h"
patch(h,"    void maxSpinChanged(double);\n",
      "    void maxSpinChanged(double);\n"
      "    void deadLowSpinChanged(double);\n"
      "    void deadHighSpinChanged(double);\n")
patch(h,"    PopupSlider *maxspinbox;\n",
      "    PopupSlider *maxspinbox;\n"
      "    PopupSlider *deadLowPopup;\n"
      "    PopupSlider *deadHighPopup;\n")

cpp="gui/src/servominmax.cpp"
patch(cpp,"#include <QtMath>\n",
      "#include <QtMath>\n#include <QSignalBlocker>\n")
patch(cpp,"    setDefaults();\n",
      """    deadLowPopup = new PopupSlider(this);
    deadLowPopup->setObjectName("deadLowPopup");
    deadLowPopup->setLimits(0,250);
    connect(deadLowPopup,SIGNAL(valueChanged(double)),this,SLOT(deadLowSpinChanged(double)));
    deadHighPopup = new PopupSlider(this);
    deadHighPopup->setObjectName("deadHighPopup");
    deadHighPopup->setLimits(0,250);
    connect(deadHighPopup,SIGNAL(valueChanged(double)),this,SLOT(deadHighSpinChanged(double)));
    setDefaults();
""")
patch(cpp,"void ServoMinMax::paintEvent(QPaintEvent *event)\n{",
      """void ServoMinMax::deadLowSpinChanged(double value)
{
    const int limited=qBound(0,qRound(value),deadLowLimit());
    if (deadLow == limited) return;
    deadLow=limited;
    update();
    emit deadLowChanged(deadLow);
}
void ServoMinMax::deadHighSpinChanged(double value)
{
    const int limited=qBound(0,qRound(value),deadHighLimit());
    if (deadHigh == limited) return;
    deadHigh=limited;
    update();
    emit deadHighChanged(deadHigh);
}

void ServoMinMax::paintEvent(QPaintEvent *event)
{""")

patch(cpp,"    maxspinbox->hide();\n\n    if(event->buttons() == Qt::LeftButton)",
      "    maxspinbox->hide();\n"
      "    deadLowPopup->hide();\n"
      "    deadHighPopup->hide();\n\n"
      "    if(event->buttons() == Qt::LeftButton)")
patch(cpp,"            deadLowStart = deadLow;\n",
      """            deadLowStart = deadLow;
            deadLowPopup->setLimits(0,deadLowLimit());
            deadLowPopup->show();
            deadLowPopup->move(this->mapToGlobal(QPoint(
                qRound(deadLowSlider.center().x())-deadLowPopup->width()/2,
                -deadLowPopup->height()+padding/2)));
            deadLowPopup->setValue(deadLow);
""")
patch(cpp,"            deadHighStart = deadHigh;\n",
      """            deadHighStart = deadHigh;
            deadHighPopup->setLimits(0,deadHighLimit());
            deadHighPopup->show();
            deadHighPopup->move(this->mapToGlobal(QPoint(
                qRound(deadHighSlider.center().x())-deadHighPopup->width()/2,
                -deadHighPopup->height()+padding/2)));
            deadHighPopup->setValue(deadHigh);
""")
patch(cpp,
      '        QToolTip::showText(event->globalPosition().toPoint(),\n'
      '            tr("Zone morte inferieure : %1 us").arg(deadLow), this);\n'
      '        update();',
      """        deadLowPopup->move(this->mapToGlobal(QPoint(
            event->pos().x()-deadLowPopup->width()/2,
            -deadLowPopup->height()+padding/2)));
        { QSignalBlocker blocker(deadLowPopup);
          deadLowPopup->setValue(deadLow); }
        update();""")
patch(cpp,
      '        QToolTip::showText(event->globalPosition().toPoint(),\n'
      '            tr("Zone morte superieure : %1 us").arg(deadHigh), this);\n'
      '        update();',
      """        deadHighPopup->move(this->mapToGlobal(QPoint(
            event->pos().x()-deadHighPopup->width()/2,
            -deadHighPopup->height()+padding/2)));
        { QSignalBlocker blocker(deadHighPopup);
          deadHighPopup->setValue(deadHigh); }
        update();""")

# Suppress the modal double-click numeric editor: a single click now opens
# the SAME yellow PopupSlider used for the center/min/max.
f=root/cpp
s=f.read_text(encoding="utf-8")
begin="void ServoMinMax::mouseDoubleClickEvent(QMouseEvent *event)\n{\n"
end="    if(event->buttons() == Qt::LeftButton) {\n"
a=s.find(begin)
b=s.find(end,a+len(begin))
if a<0 or b<0 or s.count(begin)!=1:
    raise RuntimeError("Cannot locate legacy double-click branch")
s=s[:a+len(begin)]+s[b:]
f.write_text(s,encoding="utf-8")
print("Removed legacy modal editor; the yellow PopupSlider now opens on click.")
