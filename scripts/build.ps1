# Build GameCore_Setup.exe using NSIS
# Run this script from the installer\ folder
# Requires NSIS installed: https://nsis.sourceforge.io/Download

$nsisPath = "C:\Program Files (x86)\NSIS\makensis.exe"

if (-not (Test-Path $nsisPath)) {
    Write-Host "NSIS not found at $nsisPath" -ForegroundColor Red
    Write-Host "Download from: https://nsis.sourceforge.io/Download" -ForegroundColor Yellow
    Write-Host "Also download the inetc plugin: https://nsis.sourceforge.io/Inetc_plug-in" -ForegroundColor Yellow
    exit 1
}

Write-Host "Building GameCore_Setup.exe..." -ForegroundColor Cyan
& $nsisPath "GameCore_Setup.nsi"

if ($LASTEXITCODE -eq 0) {
    Write-Host "Success! GameCore_Setup.exe created." -ForegroundColor Green
    Write-Host "Upload this to your GitHub Release assets." -ForegroundColor Yellow
} else {
    Write-Host "Build failed." -ForegroundColor Red
}
