#!/usr/bin/env python3
"""English-only GUI layout refinement, applied AFTER Expo + Deadzone patches."""
from pathlib import Path
import sys

root=Path(sys.argv[1])

def replace_once(name, old, new):
    path=root/name
    s=path.read_text(encoding="utf-8")
    count=s.count(old)
    if count!=1:
        raise RuntimeError(f"{name}: need 1 occurrence, found {count} for {old[:95]!r}")
    path.write_text(s.replace(old,new,1),encoding="utf-8")
    print("English GUI:",name)

# Keep the MAIN WINDOW visual organization intact. Only translate the existing
# top-level menu and its offline guard. Then make edited range settings in the
# modal dialog visible to the existing bar widgets BEFORE updateFromUI() reads
# their values back into the device model.
main="gui/src/mainwindow.cpp"
replace_once(main, 'menuBar()->addAction(tr("Expo / Courbes..."))',
                   'menuBar()->addAction(tr("Expo / Curves..."))')
replace_once(main, 'tr("Connecter une carte avec le firmware Expo et attendre la lecture des parametres.")',
                   'tr("Connect a board running Expo firmware and wait for the settings to load.")')
replace_once(main,
    '        dlg.applyTo(trkset);\n        updateFromUI();',
    '''        if (!dlg.applyTo(trkset)) return;
        // The MainWindow range bars are authoritative for updateFromUI().
        // Mirror ALL accepted values before asking it to send the changes.
        ui->servoPan->setMinimum(trkset.getPan_Min());
        ui->servoPan->setMaximum(trkset.getPan_Max());
        ui->servoPan->setCenter(trkset.getPan_Cnt());
        ui->servoPan->setDeadLow(trkset.getPanDZLow());
        ui->servoPan->setDeadHigh(trkset.getPanDZHigh());
        ui->servoTilt->setMinimum(trkset.getTlt_Min());
        ui->servoTilt->setMaximum(trkset.getTlt_Max());
        ui->servoTilt->setCenter(trkset.getTlt_Cnt());
        ui->servoTilt->setDeadLow(trkset.getTltDZLow());
        ui->servoTilt->setDeadHigh(trkset.getTltDZHigh());
        ui->servoRoll->setMinimum(trkset.getRll_Min());
        ui->servoRoll->setMaximum(trkset.getRll_Max());
        ui->servoRoll->setCenter(trkset.getRll_Cnt());
        ui->servoRoll->setDeadLow(trkset.getRllDZLow());
        ui->servoRoll->setDeadHigh(trkset.getRllDZHigh());
        updateFromUI();''')
# Existing popovers from Deadzone keep the tested click and drag behavior;
# only their *visible* labels change.
slider="gui/src/servominmax.cpp"
for old,new in (
    ("Zone morte inferieure : %1 us","Deadzone Left: %1 us"),
    ("Zone morte superieure : %1 us","Deadzone Right: %1 us"),
    ("Zone morte","Deadzone"),
    ("Cote inferieur (us)","Left (us)"),
    ("Cote superieur (us)","Right (us)")
):
    f=root/slider
    s=f.read_text(encoding="utf-8")
    if old in s:
        f.write_text(s.replace(old,new),encoding="utf-8")
