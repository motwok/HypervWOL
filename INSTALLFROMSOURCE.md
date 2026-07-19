# Install from Source Guide

# Build and Install from Source

### Clone the Repository

```powershell
git clone https://github.com/motwok/HypervWOL.git
cd HypervWOL
```

## Build the Project

#### Option A: Using Visual Studio

1. Open `HypervWOL.sln` in Visual Studio 2022
2. Select `Release` configuration and `x64` platform
3. Press `Ctrl+Shift+B` to build

### Option B: Using the Build Script

```powershell
.\scripts\build.ps1 -Configuration Release -Platform x64
```

### Option C: Using MSBuild Directly

```cmd
msbuild HypervWOL.sln /p:Configuration=Release /p:Platform=x64
```

## Test in Console Mode

```powershell
cd HypervWOL\x64\Release
.\HypervWOL.exe -console

# Press Ctrl+C to stop
```

## Install as a Service

```powershell
# Open PowerShell as Administrator
cd HypervWOL\x64\Release
..\..\..\scripts\install-service.ps1
```

## Check Service Status

```powershell
Get-Service HypervWOL
```

# Uninstall

```powershell
# Open PowerShell as Administrator
.\scripts\uninstall-service.ps1
```

# Troubleshooting

## Service won't start

1. Check if the executable exists at the specified path
2. Verify you have Administrator privileges
3. Check Event Viewer for error messages

## Test in console mode first

```powershell
.\HypervWOL.exe -console
```

This will help identify issues before installing as a service.
