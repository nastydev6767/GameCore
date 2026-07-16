# How to build GameCore_Setup.exe

## Requirements

1. Install **NSIS** from https://nsis.sourceforge.io/Download
2. Install the **inetc plugin** from https://nsis.sourceforge.io/Inetc_plug-in
   - Download the zip
   - Copy `inetc.dll` from the `Plugins\x86-ansi\` folder into:
     `C:\Program Files (x86)\NSIS\Plugins\x86-ansi\`

## Steps

1. Copy `icon.ico` from the GameCore root into this `installer\` folder
2. Right-click `GameCore_Setup.nsi` → Compile NSIS Script
   OR open PowerShell here and run:
   ```
   & "C:\Program Files (x86)\NSIS\makensis.exe" GameCore_Setup.nsi
   ```
3. `GameCore_Setup.exe` will appear in this folder

## Upload

Upload `GameCore_Setup.exe` to your GitHub Release at:
https://github.com/nastydev6767/GameCore/releases

The website download button points directly to it.
