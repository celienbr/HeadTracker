; HeadTracker Curve Settings - Windows installer; firmware flashing is always manual
Unicode true
!include "MUI2.nsh"
!include "x64.nsh"
Name "HeadTracker Curve Settings"
OutFile "HeadTracker-Expo-Setup.exe"
InstallDir "$PROGRAMFILES64\HeadTracker Curve Settings"
InstallDirRegKey HKLM "Software\HeadTrackerCurveSettings" "InstallDir"
RequestExecutionLevel admin
SetCompressor /SOLID lzma
ShowInstDetails show
ShowUninstDetails show
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"
Function .onInit
  ${IfNot} ${RunningX64}
    MessageBox MB_ICONSTOP "HeadTracker Curve Settings requires 64-bit Windows."
    Abort
  ${EndIf}
  SetRegView 64
  SetShellVarContext all
FunctionEnd
Function un.onInit
  SetRegView 64
  SetShellVarContext all
FunctionEnd
Section "HeadTracker Curve Settings" Main
  SetOutPath "$INSTDIR"
  File /r "package/gui/*.*"
  SetOutPath "$INSTDIR\firmware"
  File /r "package/firmware/*.*"
  SetOutPath "$INSTDIR"
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  CreateDirectory "$SMPROGRAMS\HeadTracker Curve Settings"
  CreateShortCut "$SMPROGRAMS\HeadTracker Curve Settings\HeadTracker Curve Settings.lnk" "$INSTDIR\HeadTracker.exe"
  CreateShortCut "$SMPROGRAMS\HeadTracker Curve Settings\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "Software\HeadTrackerCurveSettings" "InstallDir" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HeadTrackerCurveSettings" "DisplayName" "HeadTracker Curve Settings (modified v2.40)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HeadTrackerCurveSettings" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HeadTrackerCurveSettings" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HeadTrackerCurveSettings" "NoRepair" 1
SectionEnd
Section "Uninstall"
  Delete "$SMPROGRAMS\HeadTracker Curve Settings\HeadTracker Curve Settings.lnk"
  Delete "$SMPROGRAMS\HeadTracker Curve Settings\Uninstall.lnk"
  RMDir "$SMPROGRAMS\HeadTracker Curve Settings"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HeadTrackerCurveSettings"
  DeleteRegKey HKLM "Software\HeadTrackerCurveSettings"
  !include "uninstall-files.nsh"
SectionEnd
