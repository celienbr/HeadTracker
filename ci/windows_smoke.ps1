param([Parameter(Mandatory=$true)][string]$Directory,[string]$Report="smoke.json")
$ErrorActionPreference = 'Stop'
$Directory = (Resolve-Path $Directory).Path
$oldPath=$env:PATH
try {
  # Exclude Qt/MinGW installations to catch accidental dependence on the build host.
  $env:PATH="$env:SystemRoot\System32;$env:SystemRoot;$Directory"
  $env:QT_PLUGIN_PATH=$null
  $env:QT_QPA_PLATFORM_PLUGIN_PATH=$null
  $env:QT_QPA_PLATFORM=$null
  $p=Start-Process -FilePath "$Directory\HeadTracker.exe" -WorkingDirectory $Directory -PassThru
  $visible=$false
  for($i=0;$i -lt 40;$i++) {
    Start-Sleep -Milliseconds 250
    $p.Refresh()
    if($p.HasExited) { throw "HeadTracker exited before showing its window: $($p.ExitCode)" }
    if($p.MainWindowHandle -ne 0) {$visible=$true;break}
  }
  if(!$visible) {throw 'No visible main window'}
  Start-Sleep -Seconds 2
  $p.Refresh()
  if($p.HasExited) {throw 'Application crashed after launch'}
  $title=$p.MainWindowTitle
  if($title -notmatch 'HeadTracker') {throw "Unexpected window: $title"}
  Add-Type -AssemblyName System.Drawing
  if(-not ('HTCapture' -as [type])) {
    Add-Type @'
using System;
using System.Runtime.InteropServices;
public class HTCapture {
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left,Top,Right,Bottom; }
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT rect);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr dc, uint flags);
}
'@
  }
  $rect=New-Object HTCapture+RECT
  if(![HTCapture]::GetWindowRect($p.MainWindowHandle,[ref]$rect)) {throw 'Cannot inspect window geometry'}
  $bitmap=New-Object Drawing.Bitmap(($rect.Right-$rect.Left),($rect.Bottom-$rect.Top))
  $graphics=[Drawing.Graphics]::FromImage($bitmap)
  $dc=$graphics.GetHdc()
  $captured=[HTCapture]::PrintWindow($p.MainWindowHandle,$dc,2)
  $graphics.ReleaseHdc($dc)
  if(!$captured) {throw 'Window capture failed'}
  $bitmap.Save("$Report.png",[Drawing.Imaging.ImageFormat]::Png)
  $graphics.Dispose(); $bitmap.Dispose()
  $modules=@($p.Modules | ForEach-Object {$_.FileName})
  if(!($modules | Where-Object {$_ -like '*\platforms\qwindows.dll'})) {throw 'Qt platform plugin not loaded'}
  $null=$p.CloseMainWindow()
  if(!$p.WaitForExit(10000)) {$p.Kill(); throw 'Application did not close normally'}
  if($p.ExitCode -ne 0) {throw "Application exit code $($p.ExitCode)"}
  $boss=Start-Process -FilePath "$Directory\bossac.exe" -ArgumentList '--help' -PassThru -Wait -RedirectStandardOutput "$Report.bossac.txt"
  if($boss.ExitCode -ne 0) {throw "bossac --help failed: $($boss.ExitCode)"}
  @{result='PASS';title=$title;exitCode=$p.ExitCode;cleanPath=$true;modules=$modules;bossacHelp=$boss.ExitCode;hardwareAccess=$false} | ConvertTo-Json -Depth 4 | Set-Content $Report
} finally {$env:PATH=$oldPath}
