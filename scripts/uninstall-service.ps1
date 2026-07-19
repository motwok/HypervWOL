# Uninstall HypervWOL Windows Service
# Run this script as Administrator

param(
    [string]$ServiceName = "HypervWOL"
)

# Check if running as Administrator
$currentPrincipal = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
if (-not $currentPrincipal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    Write-Error "This script must be run as Administrator"
    exit 1
}

# Check if service exists
$service = Get-Service -Name $ServiceName -ErrorAction SilentlyContinue
if (-not $service) {
    Write-Warning "Service $ServiceName does not exist."
    exit 0
}

# Stop the service if it's running
if ($service.Status -eq 'Running') {
    Write-Host "Stopping service $ServiceName..."
    Stop-Service -Name $ServiceName -Force
}

# Delete the service
Write-Host "Removing service $ServiceName..."
sc.exe delete $ServiceName

Write-Host "Service uninstalled successfully!" -ForegroundColor Green
