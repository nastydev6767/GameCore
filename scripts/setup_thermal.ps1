# ─────────────────────────────────────────────────────────────────────────────
# FILE : scripts/setup_thermal.ps1
#
# Runs ONCE, hidden, on first launch of GameCore.
# Downloads LibreHardwareMonitorLib.dll and installs PawnIO kernel driver.
# Writes a "thermal_ready" flag file when done.
#
# Called by thermal_manager.cpp via CreateProcess with -WindowStyle Hidden.
# No window, no UAC prompt beyond the driver install (which needs elevation).
# ─────────────────────────────────────────────────────────────────────────────

$ErrorActionPreference = "Stop"

# ── Resolve paths ─────────────────────────────────────────────────────────────
$exeDir   = Split-Path -Parent $MyInvocation.MyCommand.Definition
# Scripts are in <exeDir>\scripts\, so exe dir is one up
$exeDir   = Split-Path -Parent $exeDir
$tmpDir   = [System.IO.Path]::GetTempPath()
$flagFile = Join-Path $exeDir "thermal_ready"

# Already done?
if (Test-Path $flagFile) { exit 0 }

# ── URLs ──────────────────────────────────────────────────────────────────────
# LibreHardwareMonitorLib — nightly build with PawnIO support (v0.9.x)
$lhmUrl     = "https://github.com/LibreHardwareMonitor/LibreHardwareMonitor/releases/download/v0.9.3/LibreHardwareMonitor-net472.zip"
$lhmZip     = Join-Path $tmpDir "lhm.zip"
$lhmExtract = Join-Path $tmpDir "lhm_extract"
$lhmDst     = Join-Path $exeDir "LibreHardwareMonitorLib.dll"

# PawnIO signed installer 2.2.0
$pawnUrl    = "https://github.com/namazso/PawnIO.Setup/releases/download/2.2.0/PawnIO_setup.exe"
$pawnExe    = Join-Path $tmpDir "PawnIO_setup.exe"

# ── Helper: silent download ───────────────────────────────────────────────────
function Download-File($url, $dest) {
    $wc = New-Object System.Net.WebClient
    $wc.Headers.Add("User-Agent", "GameCore/1.0")
    $wc.DownloadFile($url, $dest)
}

# ── Step 1: LibreHardwareMonitorLib.dll ──────────────────────────────────────
if (-not (Test-Path $lhmDst)) {
    try {
        Download-File $lhmUrl $lhmZip

        if (Test-Path $lhmExtract) { Remove-Item $lhmExtract -Recurse -Force }
        Expand-Archive -Path $lhmZip -DestinationPath $lhmExtract -Force

        # Find the DLL — it may be in a subfolder
        $found = Get-ChildItem -Path $lhmExtract -Recurse `
                               -Filter "LibreHardwareMonitorLib.dll" |
                 Select-Object -First 1

        if ($found) {
            Copy-Item $found.FullName $lhmDst -Force
        } else {
            # Fallback: download the raw NuGet DLL
            $nugetUrl = "https://globalcdn.nuget.org/packages/librehardwaremonitorlib.0.9.6.nupkg"
            $nupkg    = Join-Path $tmpDir "lhm.nupkg"
            Download-File $nugetUrl $nupkg
            $nupkgExtract = Join-Path $tmpDir "lhm_nupkg"
            Expand-Archive -Path $nupkg -DestinationPath $nupkgExtract -Force
            $found = Get-ChildItem -Path $nupkgExtract -Recurse `
                                   -Filter "LibreHardwareMonitorLib.dll" |
                     Select-Object -First 1
            if ($found) {
                Copy-Item $found.FullName $lhmDst -Force
            }
        }
    }
    catch {
        # Non-fatal — ThermalManager.Init() will return false gracefully
        exit 1
    }
    finally {
        if (Test-Path $lhmZip)     { Remove-Item $lhmZip     -Force -ErrorAction SilentlyContinue }
        if (Test-Path $lhmExtract) { Remove-Item $lhmExtract -Recurse -Force -ErrorAction SilentlyContinue }
    }
}

# ── Step 2: PawnIO kernel driver ──────────────────────────────────────────────
# Check if PawnIO service already exists
$pawnSvc = Get-Service -Name "PawnIO" -ErrorAction SilentlyContinue

if (-not $pawnSvc) {
    try {
        Download-File $pawnUrl $pawnExe

        # PawnIO uses NSIS — /S = silent install
        $proc = Start-Process -FilePath $pawnExe `
                              -ArgumentList "/S" `
                              -Wait -PassThru `
                              -Verb RunAs `
                              -WindowStyle Hidden

        # Give the driver a moment to register
        Start-Sleep -Seconds 2
    }
    catch {
        # PawnIO is optional — LHM can fall back to WMI-only paths
        # Don't fail the whole setup over this
    }
    finally {
        if (Test-Path $pawnExe) {
            Remove-Item $pawnExe -Force -ErrorAction SilentlyContinue
        }
    }
}

# ── Step 3: Write ready flag ───────────────────────────────────────────────────
"ok" | Out-File -FilePath $flagFile -Encoding ascii -Force

exit 0