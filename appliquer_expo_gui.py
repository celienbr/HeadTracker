#!/usr/bin/env python3
"""HeadTracker v2.40 : ajout Expo Pan, Tilt et Roll a la GUI et au firmware."""
from __future__ import annotations
import argparse
import pathlib
import subprocess
import sys

EXPO_HEADER = """// HeadTracker Expo v2.40 - courbe non lineaire par axe
#pragma once
#include <stdint.h>
static inline uint16_t expoChannel(float deltaUs, uint16_t centerUs,
                                   uint16_t minUs, uint16_t maxUs,
                                   uint8_t expoPercent)
{
    if (minUs >= maxUs) return centerUs;
    const float lo = static_cast<float>(minUs);
    const float hi = static_cast<float>(maxUs);
    float center = static_cast<float>(centerUs);
    if (center < lo) center = lo;
    if (center > hi) center = hi;
    const float span = (deltaUs < 0.0f) ? center - lo : hi - center;
    if (span <= 0.0f) return static_cast<uint16_t>(center);
    float x = deltaUs / span;
    if (x < -1.0f) x = -1.0f;
    if (x > 1.0f) x = 1.0f;
    if (expoPercent > 100) expoPercent = 100;
    const float e = static_cast<float>(expoPercent) / 100.0f;
    const float y = x * ((1.0f - e) + e * x * x);
    return static_cast<uint16_t>(center + y * span + 0.5f);
}
"""
GUI_INCLUDES = """#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QMenuBar>
#include <QSpinBox>
"""
GUI_DIALOG = """    // Expo configurable dans la GUI, enregistre en Flash via Save Settings.
    auto *expoAction = menuBar()->addAction(tr("Expo / Courbes..."));
    connect(expoAction, &QAction::triggered, this, [this]() {
        if (!serialcon->isOpen()) {
            QMessageBox::information(this, tr("HeadTracker"),
                                     tr("Connecter la carte avant de modifier l'Expo."));
            return;
        }
        QDialog dlg(this);
        dlg.setWindowTitle(tr("Courbes Expo - Pan / Tilt / Roll"));
        auto *form = new QFormLayout(&dlg);
        auto *panExpo = new QSpinBox(&dlg);
        auto *tiltExpo = new QSpinBox(&dlg);
        auto *rollExpo = new QSpinBox(&dlg);
        for (auto *spin : {panExpo, tiltExpo, rollExpo}) {
            spin->setRange(0, 100);
            spin->setSuffix(" %");
            spin->setSingleStep(5);
            spin->setToolTip(tr("0 % = lineaire ; 100 % = tres doux autour du centre."));
        }
        panExpo->setValue(trkset.getPanExpo());
        tiltExpo->setValue(trkset.getTltExpo());
        rollExpo->setValue(trkset.getRllExpo());
        form->addRow(tr("Pan (horizontal)"), panExpo);
        form->addRow(tr("Tilt (vertical)"), tiltExpo);
        form->addRow(tr("Roll (inclinaison)"), rollExpo);
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                             &dlg);
        form->addRow(buttons);
        connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        if (dlg.exec() != QDialog::Accepted) return;
        trkset.setPanExpo(static_cast<uint8_t>(panExpo->value()));
        trkset.setTltExpo(static_cast<uint8_t>(tiltExpo->value()));
        trkset.setRllExpo(static_cast<uint8_t>(rollExpo->value()));
        updateFromUI();
    });

"""
OUTPUT_OLD = """    uint16_t tiltout_ui = tiltout + trkset.getTlt_Cnt();  // Apply Center Offset
    tiltout_ui = MAX(MIN(tiltout_ui, trkset.getTlt_Max()), trkset.getTlt_Min());  // Limit Output
    uint16_t rollout_ui = rollout + trkset.getRll_Cnt();  // Apply Center Offset
    rollout_ui = MAX(MIN(rollout_ui, trkset.getRll_Max()), trkset.getRll_Min());  // Limit Output
    uint16_t panout_ui = panout + trkset.getPan_Cnt();  // Apply Center Offset
    panout_ui = MAX(MIN(panout_ui, trkset.getPan_Max()), trkset.getPan_Min());  // Limit Output
"""
OUTPUT_NEW = """    const uint16_t tiltout_ui = expoChannel(
        tiltout, trkset.getTlt_Cnt(), trkset.getTlt_Min(), trkset.getTlt_Max(),
        trkset.getTltExpo());
    const uint16_t rollout_ui = expoChannel(
        rollout, trkset.getRll_Cnt(), trkset.getRll_Min(), trkset.getRll_Max(),
        trkset.getRllExpo());
    const uint16_t panout_ui = expoChannel(
        panout, trkset.getPan_Cnt(), trkset.getPan_Min(), trkset.getPan_Max(),
        trkset.getPanExpo());
"""
ROWS = """u8,Setting,PanExpo,50,0,100,Pan Expo (percent),,,,
u8,Setting,TltExpo,35,0,100,Tilt Expo (percent),,,,
u8,Setting,RllExpo,0,0,100,Roll Expo (percent),,,,
"""

def exactly_once(text, old, new, path):
    found = text.count(old)
    if found != 1:
        raise RuntimeError(f"{path}: attendu 1 occurrence, trouve {found} ({old[:65]!r})")
    return text.replace(old, new, 1)

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("repository", type=pathlib.Path)
    args = parser.parse_args()
    repo = args.repository.resolve()
    csvfile = repo / "settings/settings.csv"
    sensefile = repo / "firmware/src/src/sense.cpp"
    headfile = repo / "firmware/src/src/expo_curve.h"
    guifile = repo / "gui/src/mainwindow.cpp"
    for f in (csvfile, sensefile, guifile):
        if not f.is_file():
            raise RuntimeError(f"Fichier manquant : {f}")
    if headfile.exists():
        raise RuntimeError("Expo deja applique : utilisez une copie propre de v2.40")
    csvtext = csvfile.read_text(encoding="utf-8")
    sensetext = sensefile.read_text(encoding="utf-8")
    guitext = guifile.read_text(encoding="utf-8")
    if "PanExpo" in csvtext or "expoChannel(" in sensetext:
        raise RuntimeError("Version deja modifiee")
    csvnew = exactly_once(csvtext,
        "float,Setting,Pan_Gain,5,MIN_GAIN,MAX_GAIN,Pan Gain,,,3,F011\n",
        "float,Setting,Pan_Gain,5,MIN_GAIN,MAX_GAIN,Pan Gain,,,3,F011\n" + ROWS,
        "settings.csv")
    sensenew = exactly_once(sensetext, '#include "sense.h"\n',
                            '#include "sense.h"\n#include "expo_curve.h"\n',
                            "sense.cpp")
    sensenew = exactly_once(sensenew, OUTPUT_OLD, OUTPUT_NEW, "sense.cpp")
    guinew = exactly_once(guitext, '#include "mainwindow.h"\n',
                          GUI_INCLUDES + '#include "mainwindow.h"\n',
                          "mainwindow.cpp")
    guinew = exactly_once(guinew, "    // Serial Connection\n",
                          GUI_DIALOG + "    // Serial Connection\n", "mainwindow.cpp")
    for f, body in ((csvfile, csvnew), (sensefile, sensenew), (guifile, guinew),
                    (headfile, EXPO_HEADER)):
        f.write_text(body, encoding="utf-8")
        print("Modifie :", f.relative_to(repo))
    subprocess.run([sys.executable, "-c",
                    "import buildguisettings; import buildfwsettings; import buildfwbtsettings"],
                   cwd=repo / "settings", check=True)
    print("Parametres GUI/FW/BLE regeneres")

if __name__ == "__main__":
    try:
        main()
    except (OSError, RuntimeError, subprocess.CalledProcessError) as exc:
        print(f"ERREUR: {exc}", file=sys.stderr)
        sys.exit(1)
