# Gamepad Cursor Mode

Gamepad Cursor Mode lets you control the mouse pointer with a gamepad while Skyrim is running. It is intended for custom mod menus that expect mouse input and provide limited gamepad support.

Press D-pad Right to turn cursor mode on or off. While the mode is active, move the pointer with the right stick. The left trigger sends a left-click. The right trigger sends a right-click.

You can change the toggle button, stick, speed, deadzone, and trigger actions in SKSE Menu Framework. The plugin still works if SKSE Menu Framework is not installed. In that case, edit `Data/SKSE/Plugins/GamepadCursorMode.json`.

## Requirements

- Skyrim Special Edition 1.6.1170
- SKSE64
- Address Library for SKSE Plugins
- A controller that is available through XInput
- SKSE Menu Framework 3 is optional and provides the in-game settings page

## Compatibility

The default toggle is D-pad Right because that button is free in the standard Complete Controller Setup layout. You can assign another button if your control map uses it.

The plugin sends normal Windows mouse input. It does not replace your control map and does not require an ESP. It is designed to complement Gamepad++, Complete Controller Setup, Prisma UI, Meridian, FLICK, Fitting Room, and other pointer-driven menus.

## Build

Install Visual Studio 2022 with the C++ workload, CMake, Git, and vcpkg. Set `VCPKG_ROOT`, then run:

```powershell
cmake --preset vs2022
cmake --build --preset release
```

The project uses CommonLibSSE-NG from the `ng` branch.

## Current status

Version 0.1.0 is an early test build. It targets Skyrim 1.6.1170. A successful build confirms the DLL compiles, but the input behavior still needs an in-game test.

## License

Gamepad Cursor Mode is available under the GNU General Public License, version 3. The vendored SKSE Menu Framework interface is also licensed under GPLv3.
