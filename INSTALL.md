# HypervWOL Installation Guide
 
## Download and extract Pre-built Binaries

1. Go to the [Releases](https://github.com/motwok/HypervWOL/releases) page
2. Download the latest release ZIP file for your platform:
   - `HypervWOL-x64.zip` for 64-bit Windows
   - `HypervWOL-Win32.zip` for 32-bit Windows
3. Extract the ZIP file to a folder (e.g., `C:\Program Files\HypervWOL\`)

## Install the Service

```powershell
# Open PowerShell as Administrator
cd "C:\Program Files\HypervWOL"

# Install the service
.\scripts\install-service.ps1 -BinaryPath "C:\Program Files\HypervWOL\HypervWOL.exe"
```

## Verify Installation

```powershell
# Check service status
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
