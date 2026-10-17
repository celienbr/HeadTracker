; HeadTracker Expo Courbes - installation Windows, sans flash automatique
Unicode true
!include "MUI2.nsh"
!include "x64.nsh"
Name "HeadTracker Expo Courbes"
OutFile "HeadTracker-Expo-Setup.exe"
InstallDir "$PROGRAMFILES64\HeadTracker Expo Courbes"
InstallDirRegKey HKLM "Software\HeadTrackerExpoGraph" "InstallDir"
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
    MessageBox MB_ICONSTOP "HeadTracker Expo Courbes necessite Windows 64 bits."
    Abort
  ${EndIf}
  SetRegView 64
  SetShellVarContext all
FunctionEnd
Function un.onInit
  SetRegView 64
  SetShellVarContext all
FunctionEnd
Section "HeadTracker Expo Courbes" Main
  SetOutPath "$INSTDIR"
  File /r "package/gui/*.*"
  SetOutPath "$INSTDIR\firmware"
  File /r "package/firmware/*.*"
  SetOutPath "$INSTDIR"
  WriteUninstaller "$INSTDIR\Desinstaller.exe"
  CreateDirectory "$SMPROGRAMS\HeadTracker Expo Courbes"
  CreateShortCut "$SMPROGRAMS\HeadTracker Expo Courbes\HeadTracker Expo Courbes.lnk" "$INSTDIR\HeadTracker.exe"
  CreateShortCut "$SMPROGRAMS\HeadTracker Expo Courbes\Desinstaller.lnk" "$INSTDIR\Desinstaller.exe"
  WriteRegStr HKLM "Software\HeadTrackerExpoGraph" "InstallDir" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HeadTrackerExpoGraph" "DisplayName" "HeadTracker Expo Courbes (v2.40 modifiee)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HeadTrackerExpoGraph" "UninstallString" '"$INSTDIR\Desinstaller.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HeadTrackerExpoGraph" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HeadTrackerExpoGraph" "NoRepair" 1
SectionEnd
Section "Uninstall"
  Delete "$SMPROGRAMS\HeadTracker Expo Courbes\HeadTracker Expo Courbes.lnk"
  Delete "$SMPROGRAMS\HeadTracker Expo Courbes\Desinstaller.lnk"
  RMDir "$SMPROGRAMS\HeadTracker Expo Courbes"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HeadTrackerExpoGraph"
  DeleteRegKey HKLM "Software\HeadTrackerExpoGraph"
  !include "uninstall-files.nsh"
SectionEnd
