# Copies built mods from this repo into the modded game's modloader folder.
# Layout: cleo/*.cs -> modloader/MyCleoMods/   plugins/*/bin/*.asi + config/*.ini -> modloader/<Plugin>/
param(
    [string]$GameDir = "C:\Games\GTA-SA-Modding"
)
$ErrorActionPreference = "Stop"
$repo = Split-Path $PSScriptRoot -Parent
$modloader = Join-Path $GameDir "modloader"

if (-not (Test-Path (Join-Path $GameDir "gta_sa.exe"))) {
    throw "gta_sa.exe (v1.0) not found in $GameDir. Finish Phase 0 first (docs/setup.md)."
}
if (-not (Test-Path $modloader)) {
    throw "modloader folder not found in $GameDir. Install Modloader first (docs/setup.md)."
}

# CLEO scripts (compiled by Sanny Builder next to their sources)
$cleoOut = Join-Path $modloader "MyCleoMods"
$scripts = Get-ChildItem (Join-Path $repo "cleo") -Filter *.cs -Recurse -ErrorAction SilentlyContinue
if ($scripts) {
    New-Item -ItemType Directory -Force $cleoOut | Out-Null
    $scripts | ForEach-Object { Copy-Item $_.FullName $cleoOut -Force; "cleo   -> $($_.Name)" }
}

# C++ plugins
Get-ChildItem (Join-Path $repo "plugins") -Directory -ErrorAction SilentlyContinue | ForEach-Object {
    $asi = Get-ChildItem $_.FullName -Filter *.asi -Recurse -ErrorAction SilentlyContinue
    if (-not $asi) { return }
    $out = Join-Path $modloader $_.Name
    New-Item -ItemType Directory -Force $out | Out-Null
    $asi | ForEach-Object { Copy-Item $_.FullName $out -Force; "plugin -> $($_.Name)" }
    $ini = Join-Path $repo "config\$($_.Name).ini"
    if (Test-Path $ini) { Copy-Item $ini $out -Force; "config -> $($_.Name).ini" }
}
"Deployed to $modloader"
