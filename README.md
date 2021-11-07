# AcrylicMenus

This is a proof-of-concept tiny application that applies acrylic effect to almost all existing Win32 context menus on Windows 10 and Windows 11 via native Win32 hooking API.

## Limitations and known issues

- Minimum supported Windows 10 version: 1903
- A little delay before applying blur effect
- Temporary lacking support of some kinds of menus (e.g. in Task Manager)
- Ugly look on Windows 10 (can be fixed by modifying msstyles file)

## Installation guideline

> **CAUTION:** It is not recommended to launch the application outside a virtual machine if you want to avoid reboot for deleteing the DLL.

The application is not stable yet, so it can't (and should not) be installed permanently.

After downloading and unzipping the archive, launch `Injector.exe` - it will inject the DLL which applies the blur effect to context menus. To unload it, press any key in the opened console window.

## Gallery

![Windows 10](https://raw.githubusercontent.com/krlvm/AcrylicMenus/master/.github/images/win10-1.png)

![Windows 10](https://raw.githubusercontent.com/krlvm/AcrylicMenus/master/.github/images/win10-2.png)

![Windows 10](https://raw.githubusercontent.com/krlvm/AcrylicMenus/master/.github/images/win10-3.png)

![Windows 10](https://raw.githubusercontent.com/krlvm/AcrylicMenus/master/.github/images/win10-4.png)

![Windows 11](https://raw.githubusercontent.com/krlvm/AcrylicMenus/master/.github/images/win11-1.png)