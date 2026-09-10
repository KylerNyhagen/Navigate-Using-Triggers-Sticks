# NUTS - Navigate Using Triggers & Sticks

Navigate Using Triggers & Sticks lets you control the mouse pointer with a gamepad while Skyrim is running. It is intended for custom mod menus that expect mouse input and provide limited (or most of the time no) gamepad support.

Open a menu, then press X to turn cursor mode on or off. The toggle does nothing during normal gameplay, I tried to make sure it only works in the menus. While the mode is active, move the pointer with the right stick. Use the left stick to scroll. The left trigger sends a left-click. The right trigger sends a right-click. Cursor mode turns off when you close the menu, but you'll probably have to just press X to toggle it off before exiting. I have plans to map B to escape, working on it.

You can change the toggle button, stick, speed, deadzone, and trigger actions in SKSE Menu Framework. The plugin still works if SKSE Menu Framework is not installed. In that case, edit `Data/SKSE/Plugins/GamepadCursorMode.json`.

## Requirements

- Skyrim Special Edition 1.6.1170 (I can't guarantee it'll work for the new update. My large modlist I use is still on 1.6.1170, and I mostly built this for myself)
- SKSE64
- Address Library for SKSE Plugins
- Auto Input Switch
- A controller that is available through XInput
- SKSE Menu Framework is optional and provides the in-game settings page

## Compatibility

The default toggle is X (The X key on your xbox controller. Not X on the keyboard. It's a gamepad mod bro), and it'll only work if you're in a menu. I picked X because, unless I'm an idiot, there doesn't seem to be anything "X" does. You can assign another button if your control map uses X for another action.

Realistically you should have Gamepad++ &  Complete Controller Setup installed if you're a controller gamer. I do, and I tested it alongside these.

 Prisma UI, Meridian, FLICK, Fitting Room, and other pointer-driven menus should work just as well. That's kinda the whole reason I built the thing. I tested fitting room because it's arguably the most complex, and half-tests FLICK in the process.

## Build

Install Visual Studio 2022 with the C++ workload, CMake, Git, and vcpkg. Set `VCPKG_ROOT`, then run:

```powershell
cmake --preset vs2022
cmake --build --preset release
```

The project uses CommonLibSSE-NG from the `ng` branch.

## Current status

Version 0.1.9 is a test build. The left stick sends mouse-wheel input while cursor mode is active. It initializes Skyrim's mouse device when cursor mode starts, so you do not need to move a physical mouse first. Cursor mode is available only while a menu is open and turns off when that menu closes. The default toggle is X. The plugin writes one log file: `GamepadCursorMode.log`.

## License

Navigate Using Triggers & Sticks is available under the GNU General Public License, version 3. The vendored SKSE Menu Framework interface is also licensed under GPLv3.
