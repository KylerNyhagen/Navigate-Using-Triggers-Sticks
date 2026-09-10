#pragma once

namespace GamepadCursorMode
{
    class CursorController final
    {
    public:
        static CursorController& GetSingleton();
        void Start();
        bool IsCursorModeActive() const;
        void SetCursorModeActive(bool a_active);

    private:
        void Run(std::stop_token a_stopToken);
        static bool IsGameWindowForeground();
        static float ApplyDeadzone(float a_value, float a_deadzone);
        static void SendMouseButton(DWORD a_flag);

        std::jthread worker_;
        std::atomic_bool active_{ false };
    };
}
