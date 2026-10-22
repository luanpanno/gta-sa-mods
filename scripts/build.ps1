# Builds every C++ plugin in plugins/ with MSBuild (VS 2022 Build Tools).
param(
    [ValidateSet("Release", "Debug")]
    [string]$Configuration = "Release"
)
$ErrorActionPreference = "Stop"
$repo = Split-Path $PSScriptRoot -Parent

if (-not $env:PLUGIN_SDK_DIR) {
    $env:PLUGIN_SDK_DIR = [Environment]::GetEnvironmentVariable("PLUGIN_SDK_DIR", "User")
}
if (-not $env:PLUGIN_SDK_DIR -or -not (Test-Path "$env:PLUGIN_SDK_DIR\output\lib\Plugin.lib")) {
    throw "plugin-sdk not found/built. See docs/setup.md (Phase 2)."
}

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$msbuild = & $vswhere -latest -products * -find "MSBuild\**\Bin\MSBuild.exe" | Select-Object -First 1
if (-not $msbuild) { throw "MSBuild not found. Install VS 2022 Build Tools (docs/setup.md)." }

Get-ChildItem (Join-Path $repo "plugins") -Filter *.vcxproj -Recurse | ForEach-Object {
    "Building $($_.BaseName) ($Configuration)..."
    & $msbuild $_.FullName /p:Configuration=$Configuration /p:Platform=Win32 /m /nologo /verbosity:minimal
    if ($LASTEXITCODE -ne 0) { throw "Build failed: $($_.Name)" }
}
