# Builds the C++ plugins, deploys all mods and launches the modded game copy (never the Steam install).
param(
    [string]$GameDir = "C:\Games\GTA-SA-Modding",
    [ValidateSet("Release", "Debug")]
    [string]$Configuration = "Release",
    [switch]$NoBuild
)
$ErrorActionPreference = "Stop"
if (Get-Process gta_sa -ErrorAction SilentlyContinue) {
    throw "The game is running. Close it first (Windows locks loaded .asi files)."
}
if (-not $NoBuild) {
    & (Join-Path $PSScriptRoot "build.ps1") -Configuration $Configuration
}
& (Join-Path $PSScriptRoot "deploy.ps1") -GameDir $GameDir -Configuration $Configuration
Start-Process -FilePath (Join-Path $GameDir "gta_sa.exe") -WorkingDirectory $GameDir
