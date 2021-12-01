# sudo-service

Sudo for windows  
**The UAC window does not appear.**  
Therefore, only run programs that are 100 percent trusted.  
It runs as a Windows service and performs pipe communication.  

## Features
- Run the program as administrator (UAC is not shown)
- Run the target program by right-clicking on the desktop.


## Quick start
Download **install.ps1** from the release page and run it.
Administrator privileges are required.
```
Set-ExecutionPolicy Unrestricted
./install.ps1
or
./install.ps1 -reg
```
## Using
Enter the command in a cmd window with normal privileges or in the Windows Run window.
(Win+R)

```
sudo cmd.exe
```
## Uninstall
Download **uninstall.ps1** from the release page and run it.
```
./uninstall.ps1
```

## Build
VIsual Studio




## TODO:
- [*] Run applications like *wt.exe*
- [ ] Flexible exception handling
- [ ] Anti virus(?)
- [ ] Change CurrentDirectory

## License
MIT
icon: https://findicons.com/icon/590712/uac
