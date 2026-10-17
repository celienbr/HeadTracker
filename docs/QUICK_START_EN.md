# HeadTracker Curve Settings — Quick Start

1. Open the project's [GitHub Releases](https://github.com/celienbr/HeadTracker/releases) and download `HeadTracker-Curve-Settings-Windows-Setup.exe`.
2. Run the installer on **Windows 64-bit**. It installs `HeadTracker Curve Settings` separately from previous editions. The installer is unsigned, so Windows may show a security warning; verify you obtained it from this repository.
3. Plug your HeadTracker into USB, refresh the COM-port list if needed, and click **Connect**. The welcome page changes to the existing configuration layout on connection.
4. Open **Curve Settings** at the top. Select Pan, Tilt or Roll and adjust Expo, deadzone left/right, center and limits. Green = active curve, solid blue = min/max, red = center, amber = deadzone, black = unused total travel. The yellow diamond appears only after output data is received.
5. Click **OK** to apply GUI edits, then **Save Settings** in the main window to persist them. Reconnect and verify values are retained.

### Arduino firmware (only if required)
- If you already have the working Expo Deadzone firmware connected, **do not reflash** just to get the new graph.
- Back up settings before any firmware programming. Choose the binary for the **physical board revision**: original Nano 33 BLE or Nano 33 BLE Rev2.
- The images are built for a **D3 recenter button**. Never use them on a D2-wired or otherwise incompatible board.
- Use **Firmware Wizard** and the appropriate firmware file. A double-click on the Arduino RESET button often restores the bootloader if required; bootloader COM port can differ.
- Verify operation on a bench with aircraft propulsion disconnected before attempting any flight.

### Rollback
Uninstall the Windows application or run the prior edition. This does not restore Arduino firmware. For the original official release visit https://github.com/headtracker/HeadTracker/releases/tag/v2.40 and check revision and D2/D3 wiring.

### File checksums
`SHA256SUMS.txt` in the Release assets lists integrity hashes. PowerShell example: `Get-FileHash .\HeadTracker-Curve-Settings-Windows-Setup.exe -Algorithm SHA256`.

### Note
The yellow marker uses received channel output. Its estimated horizontal location is **not** raw head-angle telemetry. CI tests do not replace on-hardware and preflight checks.
