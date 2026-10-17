#!/usr/bin/env python3
"""HeadTracker v2.40 : ajout Expo Pan, Tilt et Roll a la GUI et au firmware."""
from __future__ import annotations
import argparse
import pathlib
import subprocess
import sys

ASSETS = pathlib.Path(__file__).resolve().parent / "ci"
UPSTREAM_SHA = "a9ea5ca6e58a34c434aad5dc7ce641dbf03741bb"
EXPO_HEADER = (ASSETS / "expo_curve.h").read_text(encoding="utf-8")
GUI_INCLUDES = '#include "expo_dialog.h"\n'
GUI_DIALOG = """    auto *expoAction = menuBar()->addAction(tr("Expo / Courbes..."));
    expoAction->setObjectName("expoAction");
    connect(expoAction, &QAction::triggered, this, [this]() {
        if (!serialcon->isOpen() || waitingOnParameters || !trkset.supportsExpo()) {
            QMessageBox::information(this, tr("HeadTracker Expo"),
                tr("Connecter une carte avec le firmware Expo et attendre la lecture des parametres."));
            return;
        }
        ExpoDialog dlg(trkset, this);
        if (dlg.exec() != QDialog::Accepted) return;
        // A disconnect can happen while the modal dialog is open.
        if (!serialcon->isOpen() || !trkset.supportsExpo()) return;
        dlg.applyTo(trkset);
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
ROWS = """u8,Setting,PanExpo,0,0,100,Pan Expo (percent),,,,
u8,Setting,TltExpo,0,0,100,Tilt Expo (percent),,,,
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
    actual = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=repo, text=True).strip()
    if actual != UPSTREAM_SHA:
        raise RuntimeError(f"Reference incorrecte: {actual}, attendu {UPSTREAM_SHA}")
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
    subprocess.run([sys.executable, str(ASSETS / "apply_compat.py"), str(repo)], check=True)
    subprocess.run([sys.executable, str(ASSETS / "apply_deadzone.py"), str(repo)], check=True)
    subprocess.run([sys.executable, str(ASSETS / "apply_deadzone_graph.py"), str(repo)], check=True)
    subprocess.run([sys.executable, str(ASSETS / "apply_english_ui.py"), str(repo)], check=True)
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
