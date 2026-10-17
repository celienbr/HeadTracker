# Curve Settings — live graph visual update

The main HeadTracker screen keeps its original layout. On connection, HeadTracker normally replaces the Welcome pane with its existing configuration controls; this change does not alter that behavior. The new top menu is **Curve Settings**.

In the separate Curve Settings dialog:
- **Green**, solid: actual active firmware Expo and deadzone curve between the configured Min/Max positions
- **Blue**, solid vertical lines: current Min/Max limits
- **Red**, solid vertical line: center
- **Amber**, shaded: asymmetric center deadzone
- **Black**, solid: theoretical extension of the curve outside the active limits, illustrating unused total travel; not a commanded servo output
- **Yellow diamond**: current output reported by the board, positioned along the green curve; only visible after live telemetry arrives
- Black dashed reference lines have been removed entirely.

The graph shows the total supported pulse span of **988 to 2012 us**. The output value from the board supplies the diamond's Y coordinate; its X coordinate is calculated by inverting the response curve, so it is not a measurement of head angle. Flat deadzone positions cannot be uniquely inferred from output alone. The displayed diamond should not be considered a sensor-angle diagnostic.

The Arduino firmware has **not** changed from the known-good Expo Deadzone build. If that firmware is already installed and communicating, do not reflash it for these graph changes.

This version installs separately as **HeadTracker Curve Settings** to preserve the working older applications. Bench-check the full tracker system before flight.

---

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
