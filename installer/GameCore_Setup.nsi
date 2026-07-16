; GameCore Installer
; Requires NSIS + inetc plugin
; https://nsis.sourceforge.io
; https://nsis.sourceforge.io/Inetc_plug-in

!include "MUI2.nsh"
!include "LogicLib.nsh"

;--------------------------------
Name               "GameCore"
OutFile            "GameCore_Setup.exe"
InstallDir         "$PROGRAMFILES64\GameCore"
InstallDirRegKey   HKLM "Software\GameCore" "InstallDir"
RequestExecutionLevel admin
BrandingText       "GameCore v1.0  |  Free and Open Source"
SetCompressor      /SOLID lzma

;--------------------------------
VIProductVersion   "1.0.0.0"
VIAddVersionKey    "ProductName"     "GameCore"
VIAddVersionKey    "ProductVersion"  "1.0.0"
VIAddVersionKey    "CompanyName"     "nastydev6767"
VIAddVersionKey    "LegalCopyright"  "Apache 2.0 License"
VIAddVersionKey    "FileDescription" "GameCore Setup"
VIAddVersionKey    "FileVersion"     "1.0.0"

;--------------------------------
!define MUI_ABORTWARNING
!define MUI_ICON   "icon.ico"
!define MUI_UNICON "icon.ico"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_RUN           "$INSTDIR\GameCore.exe"
!define MUI_FINISHPAGE_RUN_TEXT      "Launch GameCore now"
!define MUI_FINISHPAGE_LINK          "Visit GameCore on GitHub"
!define MUI_FINISHPAGE_LINK_LOCATION "https://github.com/nastydev6767/GameCore"
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

;--------------------------------
!define DOWNLOAD_URL \
  "https://github.com/nastydev6767/GameCore/releases/latest/download/GameCore.exe"

;--------------------------------
Section "GameCore" SecMain
  SectionIn RO
  SetOutPath "$INSTDIR"

  DetailPrint "Connecting to GitHub..."
  DetailPrint "Downloading GameCore.exe..."

  inetc::get \
    /CAPTION "GameCore Setup" \
    /BANNER  "Downloading GameCore from GitHub..." \
    /RESUME  "Resume download?" \
    "${DOWNLOAD_URL}" "$INSTDIR\GameCore.exe" \
    /END

  Pop $0
  ${If} $0 != "OK"
    MessageBox MB_ICONEXCLAMATION \
      "Download failed: $0$\n$\n\
Please check your internet connection and try again.$\n$\n\
You can also download directly from:$\n\
https://github.com/nastydev6767/GameCore/releases/latest"
    Abort
  ${EndIf}

  DetailPrint "Download complete."

  File "icon.ico"
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  WriteRegStr  HKLM "Software\GameCore" "InstallDir" "$INSTDIR"
  WriteRegStr  HKLM "Software\GameCore" "Version"    "1.0.0"

  !define UK "Software\Microsoft\Windows\CurrentVersion\Uninstall\GameCore"
  WriteRegStr   HKLM "${UK}" "DisplayName"     "GameCore"
  WriteRegStr   HKLM "${UK}" "DisplayVersion"  "1.0.0"
  WriteRegStr   HKLM "${UK}" "Publisher"       "nastydev6767"
  WriteRegStr   HKLM "${UK}" "DisplayIcon"     "$INSTDIR\icon.ico"
  WriteRegStr   HKLM "${UK}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegStr   HKLM "${UK}" "URLInfoAbout"    "https://github.com/nastydev6767/GameCore"
  WriteRegDWORD HKLM "${UK}" "NoModify"        1
  WriteRegDWORD HKLM "${UK}" "NoRepair"        1
  WriteRegDWORD HKLM "${UK}" "EstimatedSize"   20480
SectionEnd

Section "Start Menu Shortcuts"
  CreateDirectory "$SMPROGRAMS\GameCore"
  CreateShortcut  "$SMPROGRAMS\GameCore\GameCore.lnk" \
                  "$INSTDIR\GameCore.exe" "" "$INSTDIR\icon.ico"
  CreateShortcut  "$SMPROGRAMS\GameCore\Uninstall GameCore.lnk" \
                  "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Desktop Shortcut"
  CreateShortcut "$DESKTOP\GameCore.lnk" \
                 "$INSTDIR\GameCore.exe" "" "$INSTDIR\icon.ico"
SectionEnd

Section "Uninstall"
  Delete "$INSTDIR\GameCore.exe"
  Delete "$INSTDIR\icon.ico"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir  "$INSTDIR"
  Delete "$SMPROGRAMS\GameCore\GameCore.lnk"
  Delete "$SMPROGRAMS\GameCore\Uninstall GameCore.lnk"
  RMDir  "$SMPROGRAMS\GameCore"
  Delete "$DESKTOP\GameCore.lnk"
  DeleteRegKey HKLM "Software\GameCore"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\GameCore"
  MessageBox MB_ICONINFORMATION "GameCore has been removed from your PC."
SectionEnd
