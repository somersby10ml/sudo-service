$APP_NAME = "sudo"
$APP_FILENAME = "$APP_NAME.exe"
$SERVICE_NAME = $APP_NAME
$SERVICE_DISPLAYNAME = "$APP_NAME service"
$INSTALL_DIR = Join-Path $HOME -ChildPath "sudo_service"
$INSTALL_FILE = Join-Path $INSTALL_DIR -ChildPath "$APP_FILENAME"

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

# Delete Service
Write-Host "Service... " -ForegroundColor "Green"
$service = Get-Service -Name $SERVICE_NAME -ErrorAction "SilentlyContinue"
if($service) {
  if($service.DisplayName -eq $SERVICE_DISPLAYNAME) { # ==

    if($service.status -eq "Running") { # ==
      Write-Host -NoNewLine "`tStop Service... " -ForegroundColor "Green"
      try {
        Stop-Service -Name $SERVICE_NAME  -ErrorAction Stop
        Write-Host "OK"
      }
      catch {
        Write-Host "failed"  -ForegroundColor "Red"
        # Write-Error $_
      }
    }

    Write-Host -NoNewLine "`tDelete Service... " -ForegroundColor "Green"
    try {
      $process = Start-Process "sc.exe" -passthru -Wait "delete $SERVICE_NAME"
      if($process.ExitCode -eq 0) {
        Write-Host "OK"
      }
      else {
        Write-Host "failed"  -ForegroundColor "Red"
        Write-Host "`tErrorCode:" $process.ExitCode -ForegroundColor "Red"
      }
    }
    catch {
      Write-Host "failed"  -ForegroundColor "Red"
      # Write-Error $_
    }
  } else {
    Write-Host "`tService check... failed" -ForegroundColor "Red"
    Write-Host "`tSThis service doesn't seem to be mine." -ForegroundColor "Red"
  }
} else {
  Write-Host "`tService Cleaned" -ForegroundColor "Green"
}

# Delete file and directory
Write-Host "File... " -ForegroundColor "Green"
if(Test-Path -Path $INSTALL_FILE) {
  try {
    Write-Host -NoNewLine "`t$INSTALL_FILE delete... " -ForegroundColor "Green"
    Remove-Item -Path $INSTALL_FILE  -ErrorAction "Stop"
    Write-Host "OK"
  }
  catch {
    Write-Host "failed"  -ForegroundColor "Red"
    Write-Error $_
  }
}
else {
  Write-Host "`t$INSTALL_FILE Cleaned" -ForegroundColor "Green"
}

if(Test-Path -Path $INSTALL_DIR) {
  try {
    Write-Host -NoNewLine "`t$INSTALL_DIR delete... " -ForegroundColor "Green"
    Remove-Item -Path $INSTALL_DIR  -ErrorAction "Stop"
    Write-Host "OK"
  }
  catch {
    Write-Host "failed"  -ForegroundColor "Red"
    Write-Error $_
  }
}
else {
  Write-Host "`t$INSTALL_DIR Cleaned" -ForegroundColor "Green"
}

# Delete Registry
Write-Host "Registry... " -ForegroundColor "Green"
$item = Get-Item -LiteralPath "Registry::HKEY_CLASSES_ROOT\*\shell\sudo\command" 
if($item) {
  Remove-Item -LiteralPath "Registry::HKEY_CLASSES_ROOT\*\shell\sudo\command"
  Write-Host "`tDelete command... OK" -ForegroundColor "Green"
}
else {
  Write-Host "`tcommand Cleaned" -ForegroundColor "Green"
}

$item = Get-Item -LiteralPath "Registry::HKEY_CLASSES_ROOT\*\shell\sudo" 
if($item) {
  Remove-Item -LiteralPath "Registry::HKEY_CLASSES_ROOT\*\shell\sudo"
  Write-Host "`tDelete sudo... OK" -ForegroundColor "Green"
}
else {
  Write-Host "`tsudo Cleaned" -ForegroundColor "Green"
}

# Delete env
$env:Path = [System.Environment]::GetEnvironmentVariable("Path", [System.EnvironmentVariableTarget]::User)
$env:Path  = ($env:Path.Split(';') | Where-Object { $_ -ne $INSTALL_DIR }) -join ';'
[System.Environment]::SetEnvironmentVariable('Path', $env:Path, [System.EnvironmentVariableTarget]::User)
Write-Host "remove sudo environment variable" -ForegroundColor "Green"

Write-Host "Uninstall was successful."