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

    void CursorController::SetCursorModeActive(bool a_active)
    {
        active_.store(a_active);
        SKSE::log::info("cursor mode {}", a_active ? "enabled" : "disabled");
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

    void CursorController::SendMouseButton(DWORD a_flag)
    {
        INPUT input{};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = a_flag;
        SendInput(1, &input, sizeof(input));
    }

    void CursorController::Run(std::stop_token a_stopToken)
    {
        using clock = std::chrono::steady_clock;
        auto previous = clock::now();
        bool toggleWasDown = false;
        bool leftWasDown = false;
        bool rightWasDown = false;

        while (!a_stopToken.stop_requested()) {
            const auto now = clock::now();
            const auto elapsed = std::chrono::duration<float>(now - previous).count();
            previous = now;

            XINPUT_STATE state{};
            const auto connected = XInputGetState(0, &state) == ERROR_SUCCESS;
            const auto settings = Config::GetSingleton().Get();
            const auto toggleDown = connected && (state.Gamepad.wButtons & settings.toggleButton) != 0;
            if (settings.enabled && toggleDown && !toggleWasDown && IsGameWindowForeground()) {
                SetCursorModeActive(!IsCursorModeActive());
            }
            toggleWasDown = toggleDown;

            if (connected && settings.enabled && IsCursorModeActive() && IsGameWindowForeground()) {
                const auto& pad = state.Gamepad;
                const auto rawX = settings.cursorStick == Stick::kLeft ? pad.sThumbLX : pad.sThumbRX;
                const auto rawY = settings.cursorStick == Stick::kLeft ? pad.sThumbLY : pad.sThumbRY;
                const auto x = ApplyDeadzone(static_cast<float>(rawX) / 32767.0F, settings.deadzone);
                const auto y = ApplyDeadzone(static_cast<float>(rawY) / 32767.0F, settings.deadzone);
                const auto dx = static_cast<LONG>(x * settings.cursorSpeed * elapsed);
                const auto dy = static_cast<LONG>(-y * settings.cursorSpeed * elapsed);
                if (dx != 0 || dy != 0) {
                    INPUT input{};
                    input.type = INPUT_MOUSE;
                    input.mi.dx = dx;
                    input.mi.dy = dy;
                    input.mi.dwFlags = MOUSEEVENTF_MOVE;
                    SendInput(1, &input, sizeof(input));
                }

                const bool leftDown = settings.leftTriggerClicks && pad.bLeftTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
                const bool rightDown = settings.rightTriggerClicks && pad.bRightTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
                if (leftDown != leftWasDown) SendMouseButton(leftDown ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP);
                if (rightDown != rightWasDown) SendMouseButton(rightDown ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP);
                leftWasDown = leftDown;
                rightWasDown = rightDown;
            } else {
                if (leftWasDown) SendMouseButton(MOUSEEVENTF_LEFTUP);
                if (rightWasDown) SendMouseButton(MOUSEEVENTF_RIGHTUP);
                leftWasDown = false;
                rightWasDown = false;
            }

            std::this_thread::sleep_for(4ms);
        }
    }
}
