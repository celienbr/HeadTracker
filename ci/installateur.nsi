; HeadTracker Expo English UI - Windows installer; firmware flashing is always manual
Unicode true
!include "MUI2.nsh"
!include "x64.nsh"
Name "HeadTracker Expo English UI"
OutFile "HeadTracker-Expo-Setup.exe"
InstallDir "$PROGRAMFILES64\HeadTracker Expo English UI"
InstallDirRegKey HKLM "Software\HeadTrackerExpoEnglish" "InstallDir"
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
    MessageBox MB_ICONSTOP "HeadTracker Expo English UI requires 64-bit Windows."
    Abort
  ${EndIf}
  SetRegView 64
  SetShellVarContext all
FunctionEnd
Function un.onInit
  SetRegView 64
  SetShellVarContext all
FunctionEnd
Section "HeadTracker Expo English UI" Main
  SetOutPath "$INSTDIR"
  File /r "package/gui/*.*"
  SetOutPath "$INSTDIR\firmware"
  File /r "package/firmware/*.*"
  SetOutPath "$INSTDIR"
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  CreateDirectory "$SMPROGRAMS\HeadTracker Expo English UI"
  CreateShortCut "$SMPROGRAMS\HeadTracker Expo English UI\HeadTracker Expo English UI.lnk" "$INSTDIR\HeadTracker.exe"
  CreateShortCut "$SMPROGRAMS\HeadTracker Expo English UI\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "Software\HeadTrackerExpoEnglish" "InstallDir" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HeadTrackerExpoEnglish" "DisplayName" "HeadTracker Expo English UI (modified v2.40)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HeadTrackerExpoEnglish" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HeadTrackerExpoEnglish" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HeadTrackerExpoEnglish" "NoRepair" 1
SectionEnd
Section "Uninstall"
  Delete "$SMPROGRAMS\HeadTracker Expo English UI\HeadTracker Expo English UI.lnk"
  Delete "$SMPROGRAMS\HeadTracker Expo English UI\Uninstall.lnk"
  RMDir "$SMPROGRAMS\HeadTracker Expo English UI"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HeadTrackerExpoEnglish"
  DeleteRegKey HKLM "Software\HeadTrackerExpoEnglish"
  !include "uninstall-files.nsh"
SectionEnd
