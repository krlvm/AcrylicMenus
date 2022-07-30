# AcrylicMenus

AcrylicMenus is a tiny application that applies acrylic effect to almost all existing Win32 context menus on Windows 10 and Windows 11 via native Win32 hooking API.

**AcrylicMenus is based on [TranslucentFlyouts](https://github.com/ALTaleX531/TranslucentFlyouts) by [@ALTaleX531](https://github.com/ALTaleX531) and [@Maplespe](https://github.com/Maplespe), which supports custom themes and aims to provide as many options for customization as possible**, while AcrylicMenus aims primarily at a standard look and feel and match the design of Windows.

## Installing

AcrylicMenus is distributed in a zip archive, which contains the injection DLL and a loader. To start AcrylicMenus, launch the `AcrylicMenusLoader.exe` - administrator rights are not required.

There's other an installer based on batch files, which can help you to install it for current user or globally with double click.

## Configuring

For finer customization, you can use [TranslucentFlyouts](https://github.com/ALTaleX531/TranslucentFlyouts), while AcrylicMenus follows a vanilla Windows design.

For minimal performance impact, all settings are hardcoded, so you need to rebuild the project to change them. They are located in `AppearanceConfiguration.h`.

Technically, AcrylicMenus supports all Win32 applications, excluding legacy and custom (ownerdrawn) popup menus, however, currently, for maximum stability and avoiding incompatibilities, it only works with File Explorer menus by default. To do this, change the parameter `INJECT_EXPLORER_ONLY` in `AcrylicMenusLoader.cpp`.

## Gallery

![Windows 10](https://raw.githubusercontent.com/krlvm/AcrylicMenus/master/github-images/win10.png)

![Windows 11](https://raw.githubusercontent.com/krlvm/AcrylicMenus/master/github-images/win11.png)