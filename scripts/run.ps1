# Deploys all mods and launches the modded game copy (never the Steam install).
param(
    [string]$GameDir = "C:\Games\GTA-SA-Modding"
)
$ErrorActionPreference = "Stop"
& (Join-Path $PSScriptRoot "deploy.ps1") -GameDir $GameDir
Start-Process -FilePath (Join-Path $GameDir "gta_sa.exe") -WorkingDirectory $GameDir
