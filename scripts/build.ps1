# Build script for HypervWOL
# Requires Visual Studio 2026 or later

param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release",
    
    [ValidateSet("Win32", "x64")]
    [string]$Platform = "x64"
)

$ErrorActionPreference = "Stop"

Write-Host "Building HypervWOL..." -ForegroundColor Cyan
Write-Host "Configuration: $Configuration" -ForegroundColor Yellow
Write-Host "Platform: $Platform" -ForegroundColor Yellow

# Find MSBuild
$msbuild = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" `
    -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe `
    | Select-Object -First 1

if (-not $msbuild) {
    Write-Error "MSBuild not found. Please install Visual Studio 2026 or later."
    exit 1
}

Write-Host "Using MSBuild: $msbuild" -ForegroundColor Gray

# Navigate to solution directory
$solutionDir = Split-Path -Parent $PSScriptRoot
Push-Location $solutionDir

try {
    # Clean
    Write-Host "Cleaning..." -ForegroundColor Cyan
    & $msbuild HypervWOL.slnx /t:Clean /p:Configuration=$Configuration /p:Platform=$Platform /v:minimal
    
    # Build
    Write-Host "Building..." -ForegroundColor Cyan
    & $msbuild HypervWOL.slnx /t:Build /p:Configuration=$Configuration /p:Platform=$Platform /v:minimal /m
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Build succeeded!" -ForegroundColor Green
        Write-Host "Output: HypervWOL\$Platform\$Configuration\HypervWOL.exe" -ForegroundColor Green
    } else {
        Write-Error "Build failed with exit code $LASTEXITCODE"
    }
} finally {
    Pop-Location
}
