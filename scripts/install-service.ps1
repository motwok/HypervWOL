# Install HypervWOL Windows Service
# Run this script as Administrator

param(
    [string]$ServiceName = "HypervWOL",
    [string]$DisplayName = "Hyper-V Wake-on-LAN Service",
    [string]$Description = "Monitors and manages Wake-on-LAN for Hyper-V virtual machines",
    [string]$BinaryPath = "$PSScriptRoot\..\HypervWOL.exe"
)

# Check if running as Administrator
$currentPrincipal = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
if (-not $currentPrincipal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    Write-Error "This script must be run as Administrator"
    exit 1
}

# Resolve full path to executable
if (Test-Path $BinaryPath) {
    $BinaryPath = Resolve-Path $BinaryPath
} else {
    Write-Error "Executable not found at: $BinaryPath"
    exit 1
}

# Check if service already exists
$existingService = Get-Service -Name $ServiceName -ErrorAction SilentlyContinue
if ($existingService) {
    Write-Warning "Service $ServiceName already exists. Stopping and removing it..."
    if ($existingService.Status -eq 'Running') {
        Stop-Service -Name $ServiceName -Force
    }
    sc.exe delete $ServiceName
    Start-Sleep -Seconds 2
}

# Create the service
Write-Host "Creating service $ServiceName..."
New-Service -Name $ServiceName `
    -BinaryPathName $BinaryPath `
    -DisplayName $DisplayName `
    -Description $Description `
    -StartupType Automatic

# Start the service
Write-Host "Starting service $ServiceName..."
Start-Service -Name $ServiceName

Write-Host "Service installed and started successfully!" -ForegroundColor Green
Write-Host "To test in console mode, run: $BinaryPath -console" -ForegroundColor Yellow
