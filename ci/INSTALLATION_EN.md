# HeadTracker Expo — English UI with curve preview

This experimental UI is derived from the working Expo Deadzone version. It is NOT an official HeadTracker v2.40 release.

## What's new
- The main HeadTracker window retains its original layout, range bars, colors, and buttons.
- Menu **Expo / Curves...** opens a larger curve editor with controls on the left and a graph on the right.
- Select **Pan**, **Tilt** or **Roll** to edit Expo %, Deadzone Left/Right, Center, Min and Max. Every axis has independent draft values.
- Blue curve: predicted output after Expo and asymmetric deadzone. Gray dashed: linear reference. Amber region: zero-output deadzone. The chart updates as you type.
- Amber deadzone handles on the original main bars retain one-click yellow numeric popovers and mouse dragging.
- **OK** applies all drafts to the software. **Cancel** discards draft changes. Then click **Save Settings** in the main window to store them on the device.
- **Restore Axis Values** discards unsaved edits for the selected axis.
- The graph is a mathematical preview, not live sensor telemetry.

## Install

Run HeadTracker-Expo-Setup.exe on Windows 64-bit. The installer creates a separate **HeadTracker Expo English UI** program and does not overwrite prior installations.

If your board already runs the working Expo Deadzone firmware, you do NOT need to reflash: the new changes are confined to the Windows UI. Confirm that the board connects and is recognized.

If firmware flashing is required, first back up settings and determine the physical Nano 33 BLE revision and recenter-button pin (this package supports **D3**). Use Firmware Wizard with the correct Nano33BLE or Nano33BLE-Rev2 file. Test on a bench without propulsion. Do not flash a second board or fly without independently verifying axes, bounds, center, power cycling, saved settings and any used PPM/SBUS/CRSF/Bluetooth link.

## Rollback

Use an earlier installed HeadTracker Expo Courbes or HeadTracker Expo Deadzone application. Windows uninstall does not alter Arduino firmware. Official v2.40 firmware and GUI: https://github.com/headtracker/HeadTracker/releases/tag/v2.40

## Automated verification

GitHub Actions builds Windows, runs Qt tests on editing, axis selection, curve rendering, deadzone handles and persistence, compiles both Arduino variants, builds NSIS and smoke-tests Windows installation and uninstall. These checks do not replace tests with real hardware.
