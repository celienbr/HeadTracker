#!/usr/bin/env python3
"""Curve Settings vNext. Keep main layout and the known-good firmware untouched."""
from pathlib import Path
import sys

root=Path(sys.argv[1])

def patch(path,old,new):
    p=root/path
    s=p.read_text(encoding="utf-8")
    if s.count(old)!=1:
        raise RuntimeError(f"Curve Settings: expected one occurrence in {path}, got {s.count(old)}")
    p.write_text(s.replace(old,new,1),encoding="utf-8")
    print("Curve Settings patch:",path)

# Menu title only; do not modify the stacked-widget behavior or reposition
# any main-window control during USB connection/disconnection.
patch("gui/src/mainwindow.cpp",
      'menuBar()->addAction(tr("Expo / Curves..."))',
      'menuBar()->addAction(tr("Curve Settings"))')
