; HeadTracker Expo - installation Windows, sans flash automatique
Unicode true
!include "MUI2.nsh"
Name "HeadTracker Expo"
OutFile "HeadTracker-Expo-Setup.exe"
InstallDir "$PROGRAMFILES64\HeadTracker Expo"
InstallDirRegKey HKLM "Software\HeadTrackerExpo" "InstallDir"
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
Section "HeadTracker Expo" Main
  SetOutPath "$INSTDIR"
  File /r "package\gui\*.*"
  SetOutPath "$INSTDIR\firmware"
  File /r "package\firmware\*.*"
  SetOutPath "$INSTDIR"
  WriteUninstaller "$INSTDIR\Desinstaller.exe"
  CreateDirectory "$SMPROGRAMS\HeadTracker Expo"
  CreateShortCut "$SMPROGRAMS\HeadTracker Expo\HeadTracker Expo.lnk" "$INSTDIR\HeadTracker.exe"
  CreateShortCut "$SMPROGRAMS\HeadTracker Expo\Desinstaller.lnk" "$INSTDIR\Desinstaller.exe"
  WriteRegStr HKLM "Software\HeadTrackerExpo" "InstallDir" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HeadTrackerExpo" "DisplayName" "HeadTracker Expo (v2.40 modifiee)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HeadTrackerExpo" "UninstallString" '"$INSTDIR\Desinstaller.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HeadTrackerExpo" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HeadTrackerExpo" "NoRepair" 1
SectionEnd
Section "Uninstall"
  Delete "$SMPROGRAMS\HeadTracker Expo\HeadTracker Expo.lnk"
  Delete "$SMPROGRAMS\HeadTracker Expo\Desinstaller.lnk"
  RMDir "$SMPROGRAMS\HeadTracker Expo"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HeadTrackerExpo"
  DeleteRegKey HKLM "Software\HeadTrackerExpo"
  RMDir /r "$INSTDIR"
SectionEnd
