; HeadTracker Expo Deadzone - installation Windows, sans flash automatique
Unicode true
!include "MUI2.nsh"
!include "x64.nsh"
Name "HeadTracker Expo Deadzone"
OutFile "HeadTracker-Expo-Setup.exe"
InstallDir "$PROGRAMFILES64\HeadTracker Expo Deadzone"
InstallDirRegKey HKLM "Software\HeadTrackerExpoDeadzone" "InstallDir"
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
!insertmacro MUI_LANGUAGE "French"
Function .onInit
  ${IfNot} ${RunningX64}
    MessageBox MB_ICONSTOP "HeadTracker Expo Deadzone necessite Windows 64 bits."
    Abort
  ${EndIf}
  SetRegView 64
  SetShellVarContext all
FunctionEnd
Function un.onInit
  SetRegView 64
  SetShellVarContext all
FunctionEnd
Section "HeadTracker Expo Deadzone" Main
  SetOutPath "$INSTDIR"
  File /r "package/gui/*.*"
  SetOutPath "$INSTDIR\firmware"
  File /r "package/firmware/*.*"
  SetOutPath "$INSTDIR"
  WriteUninstaller "$INSTDIR\Desinstaller.exe"
  CreateDirectory "$SMPROGRAMS\HeadTracker Expo Deadzone"
  CreateShortCut "$SMPROGRAMS\HeadTracker Expo Deadzone\HeadTracker Expo Deadzone.lnk" "$INSTDIR\HeadTracker.exe"
  CreateShortCut "$SMPROGRAMS\HeadTracker Expo Deadzone\Desinstaller.lnk" "$INSTDIR\Desinstaller.exe"
  WriteRegStr HKLM "Software\HeadTrackerExpoDeadzone" "InstallDir" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HeadTrackerExpoDeadzone" "DisplayName" "HeadTracker Expo Deadzone (v2.40 modifiee)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HeadTrackerExpoDeadzone" "UninstallString" '"$INSTDIR\Desinstaller.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HeadTrackerExpoDeadzone" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HeadTrackerExpoDeadzone" "NoRepair" 1
SectionEnd
Section "Uninstall"
  Delete "$SMPROGRAMS\HeadTracker Expo Deadzone\HeadTracker Expo Deadzone.lnk"
  Delete "$SMPROGRAMS\HeadTracker Expo Deadzone\Desinstaller.lnk"
  RMDir "$SMPROGRAMS\HeadTracker Expo Deadzone"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HeadTrackerExpoDeadzone"
  DeleteRegKey HKLM "Software\HeadTrackerExpoDeadzone"
  !include "uninstall-files.nsh"
SectionEnd
