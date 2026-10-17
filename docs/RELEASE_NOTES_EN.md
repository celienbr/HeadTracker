# HeadTracker v2.40 — Curve Settings Edition

**Unofficial community preview — 17 October 2026**

### Main features
- Independently adjustable Pan, Tilt and Roll Expo (0–100%).
- Independent left/right deadzone of 0–250 µs per axis, editable by dragging or clicking the yellow numeric popover.
- English **Curve Settings** dialog; green actual active curve, solid blue travel limits, red center, amber deadzone, black extrapolated unused travel, and yellow live output marker.
- Familiar main-window layout retained. Settings can be written to device flash using Save Settings.

### Assets
- `HeadTracker-Curve-Settings-Windows-Setup.exe`: Windows 64-bit installer (recommended).
- `HeadTracker-Curve-Settings-Windows-Portable.zip`: portable Windows program with runtime DLLs.
- `HeadTracker-Curve-Settings-Nano33BLE.bin`: original Arduino Nano 33 BLE firmware.
- `HeadTracker-Curve-Settings-Nano33BLE-Rev2.bin`: Nano 33 BLE Rev2 firmware.
- `SHA256SUMS.txt`: SHA-256 file hashes to check downloads.
- `QUICK_START_EN.md`, `BUILD_PROVENANCE.txt`, `THIRD_PARTY_NOTICES.md`, `LICENSE-LGPL-3.0.txt`, and source code archives.

### Important limitations
Firmware builds target the **D3 recenter-button wiring**; do not flash a D2-wired or unverified board. Identify the correct Arduino hardware revision. The installer does not program hardware. Back up settings and verify all relevant outputs, USB connections and links before flying. Automated native, Qt, firmware and Windows installation tests passed, but real hardware configurations and flight behavior have not been broadly certified. Use at your own risk on a bench first.

The yellow diamond follows board-reported **channel output**; its horizontal graph position is estimated, not a direct measurement of head motion.

### Credits and source
Community modification, **not an official HeadTracker release or endorsement**. Upstream v2.40 source pinned at `a9ea5ca6e58a34c434aad5dc7ce641dbf03741bb`; source patches and build recipe are in the release tag. Project changes remain under GNU GPLv3. See the license and third-party notices.

[English installation instructions](https://github.com/celienbr/HeadTracker/blob/curve-settings-live-graph/docs/QUICK_START_EN.md)
