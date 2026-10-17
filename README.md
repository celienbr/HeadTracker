# HeadTracker v2.40 — Curve Settings Edition

**Unofficial community modification** of [HeadTracker v2.40](https://github.com/headtracker/HeadTracker/releases/tag/v2.40).

**Download:** [Public GitHub Releases](https://github.com/celienbr/HeadTracker/releases). Choose the community preview release and the Windows Setup EXE.

### Features
- Independent Pan, Tilt and Roll Expo (0–100%) and asymmetric left/right center deadzone (0–250 µs per side).
- Click or drag the amber deadzone handles in the original main layout.
- English **Curve Settings** dialog with a green output curve, solid blue limits, red center, amber deadzone and black unused travel. Yellow diamond tracks reported output after USB connection.
- No Arduino flashing required to use the new GUI if you already have the compatible Expo Deadzone firmware working.

### Downloads and documentation
- Windows x64 installer, portable Windows ZIP, and two separate Arduino Nano 33 BLE binaries are provided under Releases.
- [Quick Start (English)](docs/QUICK_START_EN.md), [Release Notes](docs/RELEASE_NOTES_EN.md), [Third-party notices](THIRD_PARTY_NOTICES.md).
- The source code for each release is available from its GitHub tag, together with the pinned upstream-commit patch scripts.

### Hardware and safety
The firmware images are for **original Nano 33 BLE** and **Nano 33 BLE Rev2** respectively, with the **D3 recenter button** configuration. A D2-wired tracker needs its own compatible firmware. Flashing is always a manual, explicit action. Back up existing settings and verify all outputs and reconnection on a bench before use in flight. Automated testing is **not certification** or validation of every hardware variant. The yellow diamond's X position is inferred from output, not measured head angle.

### Source and license
The default branch **master** preserves the unmodified upstream fork. The maintained community branch is [curve-settings-live-graph](https://github.com/celienbr/HeadTracker/tree/curve-settings-live-graph). The [GitHub Actions build](.github/workflows/headtracker-expo-release.yml) applies modifications to official v2.40 upstream commit `a9ea5ca6e58a34c434aad5dc7ce641dbf03741bb` and builds firmware and the Windows GUI.

Project and modifications: **GNU GPLv3** — [LICENSE](LICENSE). Qt 6 runtime and other libraries have separate license requirements — [Third-party notices](THIRD_PARTY_NOTICES.md). Original HeadTracker developers have not endorsed or certified this fork.

Please use [Issues](https://github.com/celienbr/HeadTracker/issues) to report bugs or tested hardware combinations.
