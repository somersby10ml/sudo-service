param (
  [switch]$reg = $false
)

function Get-Administrator {  
  $user = [Security.Principal.WindowsIdentity]::GetCurrent();
  return (New-Object Security.Principal.WindowsPrincipal $user).IsInRole([Security.Principal.WindowsBuiltinRole]::Administrator)  
}

if(-not (Get-Administrator)) {
  Write-Host "Administrator privileges are required to install." -ForegroundColor "Yellow"
    Write-Host "Are you sure you want to run it again with administrator privileges? [y/n]"
    $key = [Console]::ReadKey()
    Write-Host
    if($key.KeyChar -eq "y") {
      Write-Host "Run again as administrator"
      Start-Process powershell.exe "-NoExit -NoProfile -ExecutionPolicy Bypass -File `"$PSCommandPath`"" -Verb RunAs;
    } else {
      Write-Host "Exit..."
    }
    exit 1;
}


$APP_NAME = "sudo"
$APP_FILENAME = "$APP_NAME.exe"

$SERVICE_NAME = $APP_NAME
$SERVICE_DISPLAYNAME = "$APP_NAME service"
$SERVICE_DESCRIPTION = "Run programs with administrator privileges instead."

$INSTALL_URL = "https://github.com/somersby10ml/sudo-service/releases/latest/download/sudo.exe"
$INSTALL_DIR = Join-Path $HOME -ChildPath "sudo_service"
$INSTALL_FILE = Join-Path $INSTALL_DIR -ChildPath "$APP_FILENAME"

# Service Check
$service = Get-Service -Name $SERVICE_NAME -ErrorAction "SilentlyContinue"
if($service) {
  
  if($service.DisplayName -eq $SERVICE_DISPLAYNAME) { # ==
    if($service.status -ne "Running") { # != 
      Write-Host -NoNewLine "Start Service... " -ForegroundColor "Green"
      try {
        Start-Service -Name $SERVICE_NAME  -ErrorAction Stop
        Write-Host "OK"
      }
      catch {
        Write-Host "failed"  -ForegroundColor "Red"
        Write-Error $_
      }
    }
    Write-Host "It is already installed." -ForegroundColor "Green"
  } else {
    Write-Host "Service check... failed" -ForegroundColor "Red"
    Write-Host "Some services already exist. The installation could not be completed." -ForegroundColor "Red"
    Write-Host "You must uninstall the program or uninstall the service before installing." -ForegroundColor "Red"
  }
  exit 1
}

# Create directory if it does not exist
if(-not(Test-Path -Path $INSTALL_DIR)) {
  Write-Host -NoNewLine "Create Directory ($INSTALL_DIR) ... " -ForegroundColor "Green"
  New-Item -ItemType Director -Path $INSTALL_DIR -ErrorAction "Stop" | Out-Null
  Write-Host "OK"
}

# Add user environment variable
$env:Path = [System.Environment]::GetEnvironmentVariable("Path", [System.EnvironmentVariableTarget]::User)
$exist = ($env:Path.Split(';') | Where-Object { $_ -eq $INSTALL_DIR })
if(-not($exist)) {
  Write-Host -NoNewLine "Add environment variable... " -ForegroundColor "Green"
  $env:Path += ";${INSTALL_DIR}"
  [System.Environment]::SetEnvironmentVariable('Path', $env:Path, [System.EnvironmentVariableTarget]::User)
  Write-Host "OK"
}

# File download latest version 
try {
  Write-Host -NoNewLine "Download sudo... " -ForegroundColor "Green"
  $webClient = New-Object System.Net.WebClient
  $webClient.DownloadFile($INSTALL_URL, $INSTALL_FILE)
  Write-Host "OK"
}
catch {
  Write-Host "failed" -ForegroundColor "Red"
  Write-Host $_
  exit 1
}

# check download file
if(-not (Test-Path -Path $INSTALL_FILE -PathType Leaf)) {
  Write-Host "$APP_FILENAME not found" -ForegroundColor "Red"
  exit 1
}

# Add Service
$service = Get-Service -Name $SERVICE_NAME -ErrorAction SilentlyContinue
if(-not($service)) {
  try {
    Write-Host -NoNewLine "Add Service... " -ForegroundColor "Green"
    $params = @{
      Name = $SERVICE_NAME
      BinaryPathName = $INSTALL_FILE
      StartupType = "Automatic"
      DisplayName = $SERVICE_DISPLAYNAME
      Description = $SERVICE_DESCRIPTION
    }
    New-Service @params  -ErrorAction "Stop" | Out-Null
    Write-Host "OK"
  }
  catch {
    Write-Host "failed" -ForegroundColor "Red"
    Write-Host $_
    exit 1
  }
}

# Start Service
Write-Host -NoNewLine "Start Service... " -ForegroundColor "Green"
try {
  Start-Service -Name $SERVICE_NAME  -ErrorAction Stop
  Write-Host "OK"
}
catch {
  Write-Host "failed"  -ForegroundColor "Red"
  Write-Error $_
}


if($reg) {
  Write-Host -NoNewLine "Add right-click menu... " -ForegroundColor "Green"
  try {
    New-Item 'Registry::HKEY_CLASSES_ROOT\*\shell\sudo' -ErrorAction SilentlyContinue | Out-Null 
    New-Item 'Registry::HKEY_CLASSES_ROOT\*\shell\sudo\command' -ErrorAction SilentlyContinue | Out-Null
    New-ItemProperty -LiteralPath 'Registry::HKEY_CLASSES_ROOT\*\shell\sudo' -Name 'Icon' -Value """$INSTALL_FILE""" -ErrorAction SilentlyContinue | Out-Null
    New-ItemProperty -LiteralPath 'Registry::HKEY_CLASSES_ROOT\*\shell\sudo' -Name 'Position' -Value 'Top' -ErrorAction SilentlyContinue | Out-Null
    New-ItemProperty -LiteralPath 'Registry::HKEY_CLASSES_ROOT\*\shell\sudo\command' -Name '(Default)' -Value """$INSTALL_FILE"" ""%1"""  -ErrorAction SilentlyContinue | Out-Null
    Write-Host "OK"
  }
  catch {
    Write-Host "failed"  -ForegroundColor "Red"
    Write-Error $_
  }
}
Write-Host "Installation was successful."  -ForegroundColor "Green"


