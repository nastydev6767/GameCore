; GameCore Installer Script
; Built with NSIS (Nullsoft Scriptable Install System)
; https://nsis.sourceforge.io
; To compile: makensis GameCore_Setup.nsi

!include "MUI2.nsh"
!include "LogicLib.nsh"

;--------------------------------
; General
Name               "GameCore"
OutFile            "GameCore_Setup.exe"
InstallDir         "$PROGRAMFILES64\GameCore"
InstallDirRegKey   HKLM "Software\GameCore" "InstallDir"
RequestExecutionLevel admin
BrandingText       "GameCore v1.0  |  Free and Open Source"
SetCompressor      /SOLID lzma

;--------------------------------
; Version info
VIProductVersion   "1.0.0.0"
VIAddVersionKey    "ProductName"     "GameCore"
VIAddVersionKey    "ProductVersion"  "1.0.0"
VIAddVersionKey    "CompanyName"     "nastydev6767"
VIAddVersionKey    "LegalCopyright"  "Apache 2.0 License"
VIAddVersionKey    "FileDescription" "GameCore Setup"
VIAddVersionKey    "FileVersion"     "1.0.0"

;--------------------------------
; UI
!define MUI_ABORTWARNING
!define MUI_ICON   "..\icon.ico"
!define MUI_UNICON "..\icon.ico"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

; Finish page — checkbox to launch
!define MUI_FINISHPAGE_RUN              "$INSTDIR\GameCore.exe"
!define MUI_FINISHPAGE_RUN_TEXT         "Launch GameCore now"
!define MUI_FINISHPAGE_LINK             "Visit GameCore on GitHub"
!define MUI_FINISHPAGE_LINK_LOCATION    "https://github.com/nastydev6767/GameCore"
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

;--------------------------------
; GitHub download URL
!define DOWNLOAD_URL \
  "https://github.com/nastydev6767/GameCore/releases/latest/download/GameCore.exe"

;--------------------------------
; Main install section
Section "GameCore" SecMain
  SectionIn RO

  SetOutPath "$INSTDIR"

  DetailPrint "Connecting to GitHub Releases..."
  DetailPrint "Downloading GameCore.exe..."

  ; Download GameCore.exe from latest GitHub Release
  ; inetc plugin handles progress bar automatically
  inetc::get \
    /CAPTION "GameCore Setup" \
    /BANNER  "Downloading GameCore..." \
    /RESUME  "Resume download?" \
    "${DOWNLOAD_URL}" "$INSTDIR\GameCore.exe" \
    /END

  Pop $0
  ${If} $0 != "OK"
    MessageBox MB_ICONEXCLAMATION \
      "Download failed ($0).$\n$\n\
Please check your internet connection and try again.$\n$\n\
You can also download manually from:$\n\
https://github.com/nastydev6767/GameCore/releases/latest"
    Abort
  ${EndIf}

  DetailPrint "GameCore.exe downloaded successfully."

  ; Copy icon
  File "..\icon.ico"

  ; Write uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ; Registry
  WriteRegStr HKLM "Software\GameCore" "InstallDir" "$INSTDIR"
  WriteRegStr HKLM "Software\GameCore" "Version"    "1.0.0"

  ; Add/Remove Programs
  !define UNINSTALL_KEY \
    "Software\Microsoft\Windows\CurrentVersion\Uninstall\GameCore"
  WriteRegStr   HKLM "${UNINSTALL_KEY}" "DisplayName"     "GameCore"
  WriteRegStr   HKLM "${UNINSTALL_KEY}" "DisplayVersion"  "1.0.0"
  WriteRegStr   HKLM "${UNINSTALL_KEY}" "Publisher"       "nastydev6767"
  WriteRegStr   HKLM "${UNINSTALL_KEY}" "DisplayIcon"     "$INSTDIR\icon.ico"
  WriteRegStr   HKLM "${UNINSTALL_KEY}" "UninstallString" \
    '"$INSTDIR\Uninstall.exe"'
  WriteRegStr   HKLM "${UNINSTALL_KEY}" "URLInfoAbout" \
    "https://github.com/nastydev6767/GameCore"
  WriteRegDWORD HKLM "${UNINSTALL_KEY}" "NoModify"        1
  WriteRegDWORD HKLM "${UNINSTALL_KEY}" "NoRepair"        1
  WriteRegDWORD HKLM "${UNINSTALL_KEY}" "EstimatedSize"   15360

SectionEnd

Section "Start Menu Shortcuts" SecStart
  CreateDirectory "$SMPROGRAMS\GameCore"
  CreateShortcut  "$SMPROGRAMS\GameCore\GameCore.lnk" \
                  "$INSTDIR\GameCore.exe" "" "$INSTDIR\icon.ico" 0
  CreateShortcut  "$SMPROGRAMS\GameCore\Uninstall.lnk" \
                  "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Desktop Shortcut" SecDesktop
  CreateShortcut "$DESKTOP\GameCore.lnk" \
                 "$INSTDIR\GameCore.exe" "" "$INSTDIR\icon.ico" 0
SectionEnd

;--------------------------------
; Uninstaller
Section "Uninstall"
  Delete "$INSTDIR\GameCore.exe"
  Delete "$INSTDIR\icon.ico"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir  "$INSTDIR"

  Delete "$SMPROGRAMS\GameCore\GameCore.lnk"
  Delete "$SMPROGRAMS\GameCore\Uninstall.lnk"
  RMDir  "$SMPROGRAMS\GameCore"

  Delete "$DESKTOP\GameCore.lnk"

  DeleteRegKey HKLM "Software\GameCore"
  DeleteRegKey HKLM \
    "Software\Microsoft\Windows\CurrentVersion\Uninstall\GameCore"

  MessageBox MB_ICONINFORMATION "GameCore has been removed from your PC."
SectionEnd
