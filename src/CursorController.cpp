#include "CursorController.h"
#include "Config.h"

namespace GamepadCursorMode
{
    CursorController& CursorController::GetSingleton()
    {
        static CursorController instance;
        return instance;
    }

    void CursorController::Start()
    {
        if (!worker_.joinable()) {
            worker_ = std::jthread([this](std::stop_token token) { Run(token); });
        }
    }

    bool CursorController::IsCursorModeActive() const { return active_.load(); }
    bool CursorController::ShouldSuppressGamepadInput() const { return suppressGamepadInput_.load(); }
    bool CursorController::IsMenuAvailable() const { return menuAvailable_.load(); }
    void CursorController::SetMenuAvailable(bool a_available) { menuAvailable_.store(a_available); }

    void CursorController::SetCursorModeActive(bool a_active)
    {
        active_.store(a_active);
        if (a_active) suppressGamepadInput_.store(true);
        SKSE::log::info("cursor mode {}", a_active ? "enabled" : "disabled");
        SetSkyrimCursorActive(a_active);

        if (a_active) {
            SKSE::GetTaskInterface()->AddTask([]() {
                if (auto* manager = RE::BSInputDeviceManager::GetSingleton()) {
                    manager->ReinitializeMouse();
                    SKSE::log::info("Skyrim mouse device reinitialized for cursor mode");
                }
            });
            // Give Auto Input Switch and the open menu a mouse event after the device
            // is initialized. This removes the need to move a physical mouse first.
            // For those peeking through, I found that if I didn't do this, I had to jiggle my mouse in order for the plugin to work lol.
            QueueMouseMove(1, 0);
        } else {
            // Drop any final cursor-mode delta before gameplay resumes. Without
            // this reset Skyrim can apply that delta to the player camera.
            SKSE::GetTaskInterface()->AddTask([]() {
                if (auto* manager = RE::BSInputDeviceManager::GetSingleton()) {
                    manager->ReinitializeMouse();
                }
            });
        }
    }

    void CursorController::SetSkyrimCursorActive(bool a_active)
    {
        SKSE::GetTaskInterface()->AddUITask([a_active]() {
            // Otherwise your windows mouse pops up in front of Skyrim and looks ugle.
            if (auto* cursor = RE::MenuCursor::GetSingleton()) {
                cursor->SetCursorVisibility(false);
            }

            const auto* strings = RE::InterfaceStrings::GetSingleton();
            auto* messages = RE::UIMessageQueue::GetSingleton();
            if (!strings || !messages) {
                SKSE::log::error("could not {} Skyrim's Cursor Menu", a_active ? "show" : "hide");
                return;
            }

            auto message = a_active ? RE::UI_MESSAGE_TYPE::kShow : RE::UI_MESSAGE_TYPE::kHide;
            if (a_active) {
                if (auto* ui = RE::UI::GetSingleton(); ui && ui->IsMenuOpen(strings->cursorMenu)) {
                    message = RE::UI_MESSAGE_TYPE::kUpdate;
                }
            }
            messages->AddMessage(strings->cursorMenu, message, nullptr);
            SKSE::log::info("Skyrim Cursor Menu {} message queued", a_active ? "show/update" : "hide");
        });
    }

    bool CursorController::IsGameWindowForeground()
    {
        const auto window = GetForegroundWindow();
        if (!window) {
            return false;
        }
        DWORD processID = 0;
        GetWindowThreadProcessId(window, &processID);
        return processID == GetCurrentProcessId();
    }

    float CursorController::ApplyDeadzone(float a_value, float a_deadzone)
    {
        const auto magnitude = std::abs(a_value);
        if (magnitude <= a_deadzone) {
            return 0.0F;
        }
        const auto scaled = (magnitude - a_deadzone) / (1.0F - a_deadzone);
        return std::copysign(scaled * scaled, a_value);
    }

    void CursorController::QueueMouseMove(LONG a_dx, LONG a_dy)
    {
        POINT screenPosition{};
        if (GetCursorPos(&screenPosition)) {
            SetCursorPos(screenPosition.x + a_dx, screenPosition.y + a_dy);
        }

        pendingX_.fetch_add(a_dx);
        pendingY_.fetch_add(a_dy);
        if (moveTaskPending_.exchange(true)) {
            return;
        }

        SKSE::GetTaskInterface()->AddTask([this]() {
            const auto dx = pendingX_.exchange(0);
            const auto dy = pendingY_.exchange(0);
            moveTaskPending_.store(false);

            if (!IsCursorModeActive()) {
                return;
            }
            if (auto* queue = RE::BSInputEventQueue::GetSingleton()) {
                queue->AddMouseMoveEvent(static_cast<std::int32_t>(dx), static_cast<std::int32_t>(dy));
            }
        });
    }

    void CursorController::QueueMouseButton(std::uint32_t a_button, bool a_down)
    {
        SKSE::GetTaskInterface()->AddTask([a_button, a_down]() {
            if (auto* queue = RE::BSInputEventQueue::GetSingleton()) {
                queue->AddButtonEvent(RE::INPUT_DEVICE::kMouse, static_cast<std::int32_t>(a_button),
                    a_down ? 1.0F : 0.0F, a_down ? 0.0F : 0.01F);
            }
        });
    }

    void CursorController::QueueMouseWheel(std::int32_t a_notches)
    {
        if (a_notches == 0) {
            return;
        }
        // Skyrim represents the wheel as mouse button events. I just mapped them to "notches".
        a_notches = std::clamp(a_notches, -3, 3);
        SKSE::GetTaskInterface()->AddTask([a_notches]() {
            auto* queue = RE::BSInputEventQueue::GetSingleton();
            if (!queue) {
                return;
            }
            const auto id = a_notches > 0 ? RE::BSWin32MouseDevice::Keys::kWheelUp : RE::BSWin32MouseDevice::Keys::kWheelDown;
            for (std::int32_t i = 0; i < std::abs(a_notches); ++i) {
                queue->AddButtonEvent(RE::INPUT_DEVICE::kMouse, static_cast<std::int32_t>(id), 1.0F, 0.0F);
            }
        });
    }

    void CursorController::QueueKeyboardButton(std::uint32_t a_key, bool a_down)
    {
        SKSE::GetTaskInterface()->AddTask([a_key, a_down]() {
            if (auto* queue = RE::BSInputEventQueue::GetSingleton()) {
                queue->AddButtonEvent(RE::INPUT_DEVICE::kKeyboard, static_cast<std::int32_t>(a_key),
                    a_down ? 1.0F : 0.0F, a_down ? 0.0F : 0.01F);
            }
        });
    }

    void CursorController::Run(std::stop_token a_stopToken)
    {
        using clock = std::chrono::steady_clock;
        auto previous = clock::now();
        bool toggleWasDown = false;
        bool leftWasDown = false;
        bool rightWasDown = false;
        bool escapeWasDown = false;
        int activeController = -1;
        auto nextStatusLog = clock::now();
        auto nextMovementLog = clock::now();
        auto nextScrollLog = clock::now();
        float scrollRemainder = 0.0F;
        auto lastToggle = clock::now() - 1s;

        SKSE::log::info("controller polling thread started; scanning XInput slots 0 through 3");

        while (!a_stopToken.stop_requested()) {
            const auto now = clock::now();
            const auto elapsed = std::chrono::duration<float>(now - previous).count();
            previous = now;

            XINPUT_STATE state{};
            int detectedController = -1;
            for (DWORD index = 0; index < XUSER_MAX_COUNT; ++index) {
                XINPUT_STATE candidate{};
                if (XInputGetState(index, &candidate) == ERROR_SUCCESS) {
                    detectedController = static_cast<int>(index);
                    state = candidate;
                    break;
                }
            }
            const auto connected = detectedController >= 0;
            // As someone with multiple controllers around the house, and sometimes my kid turns them on, I figured I'd just check all the controller slots so the one in my basement isn't randomly on as "controller 1"
            if (detectedController != activeController) {
                activeController = detectedController;
                if (activeController >= 0) {
                    SKSE::log::info("XInput controller detected in slot {}", activeController);
                } else {
                    SKSE::log::warn("no XInput controller detected");
                }
            }
            const auto settings = Config::GetSingleton().Get();
            const auto toggleDown = connected && (state.Gamepad.wButtons & settings.toggleButton) != 0;
            const auto escapeDown = connected && (state.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0;
            if (IsCursorModeActive() && !IsMenuAvailable()) {
                SKSE::log::info("cursor mode disabled because no eligible menu is open");
                SetCursorModeActive(false);
            }
            if (settings.enabled && IsMenuAvailable() && toggleDown && !toggleWasDown &&
                IsGameWindowForeground() && now - lastToggle >= 300ms) {
                SKSE::log::info("toggle press detected: slot={}, buttons=0x{:04X}, configuredMask=0x{:04X}",
                    activeController, state.Gamepad.wButtons, settings.toggleButton);
                SetCursorModeActive(!IsCursorModeActive());
                lastToggle = now;
            }
            toggleWasDown = toggleDown;
            if (!IsCursorModeActive() && !toggleDown && !escapeDown) {
                suppressGamepadInput_.store(false);
            }

            if (now >= nextStatusLog) {
                SKSE::log::info(
                    "status: connected={}, slot={}, foreground={}, enabled={}, active={}, buttons=0x{:04X}, left=({}, {}), right=({}, {}), triggers=({}, {})",
                    connected, activeController, IsGameWindowForeground(), settings.enabled, IsCursorModeActive(),
                    state.Gamepad.wButtons, state.Gamepad.sThumbLX, state.Gamepad.sThumbLY,
                    state.Gamepad.sThumbRX, state.Gamepad.sThumbRY,
                    state.Gamepad.bLeftTrigger, state.Gamepad.bRightTrigger);
                nextStatusLog = now + 5s;
            }

            if (connected && settings.enabled && IsCursorModeActive() && IsGameWindowForeground()) {
                if (escapeDown && !escapeWasDown) {
                    QueueKeyboardButton(RE::BSKeyboardDevice::Keys::kEscape, true);
                    QueueKeyboardButton(RE::BSKeyboardDevice::Keys::kEscape, false);
                    SKSE::log::info("B mapped to Escape; cursor mode disabled before menu close");
                    escapeWasDown = true;
                    SetCursorModeActive(false);
                    continue;
                }
                const auto& pad = state.Gamepad;
                const auto rawX = settings.cursorStick == Stick::kLeft ? pad.sThumbLX : pad.sThumbRX;
                const auto rawY = settings.cursorStick == Stick::kLeft ? pad.sThumbLY : pad.sThumbRY;
                const auto x = ApplyDeadzone(static_cast<float>(rawX) / 32767.0F, settings.deadzone);
                const auto y = ApplyDeadzone(static_cast<float>(rawY) / 32767.0F, settings.deadzone);
                const auto dx = static_cast<LONG>(x * settings.cursorSpeed * elapsed);
                const auto dy = static_cast<LONG>(-y * settings.cursorSpeed * elapsed);
                if (dx != 0 || dy != 0) {
                    QueueMouseMove(dx, dy);
                    if (now >= nextMovementLog) {
                        POINT cursor{};
                        GetCursorPos(&cursor);
                        SKSE::log::info("hybrid cursor movement: dx={}, dy={}, screen=({}, {})", dx, dy, cursor.x, cursor.y);
                        nextMovementLog = now + 1s;
                    }
                }

                // The left stick is an independent scroll wheel. Accumulation makes
                // small stick movements smooth while still producing discrete wheel
                // notches for Scaleform and custom menu frameworks.
                const auto scrollInput = ApplyDeadzone(static_cast<float>(pad.sThumbLY) / 32767.0F, settings.deadzone);
                scrollRemainder += scrollInput * elapsed * 10.0F;
                auto notches = static_cast<std::int32_t>(scrollRemainder);
                if (notches != 0) {
                    notches = std::clamp(notches, -3, 3);
                    scrollRemainder -= static_cast<float>(notches);
                    QueueMouseWheel(notches);
                    if (now >= nextScrollLog) {
                        SKSE::log::info("left-stick scroll: notches={}", notches);
                        nextScrollLog = now + 1s;
                    }
                }
                // Logging for the triggers.
                const bool leftDown = settings.leftTriggerClicks && pad.bLeftTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
                const bool rightDown = settings.rightTriggerClicks && pad.bRightTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
                if (leftDown != leftWasDown) {
                    QueueMouseButton(0, leftDown);
                    SKSE::log::info("native left-click {} queued", leftDown ? "down" : "up");
                }
                if (rightDown != rightWasDown) {
                    QueueMouseButton(1, rightDown);
                    SKSE::log::info("native right-click {} queued", rightDown ? "down" : "up");
                }
                leftWasDown = leftDown;
                rightWasDown = rightDown;
                escapeWasDown = escapeDown;
            } else {
                scrollRemainder = 0.0F;
                if (leftWasDown) QueueMouseButton(0, false);
                if (rightWasDown) QueueMouseButton(1, false);
                leftWasDown = false;
                rightWasDown = false;
                if (!escapeDown) {
                    escapeWasDown = false;
                }
            }

            std::this_thread::sleep_for(4ms);
        }
    }
}
