#pragma once

namespace GamepadCursorMode
{
    class CursorController final
    {
    public:
        static CursorController& GetSingleton();
        void Start();
        bool IsCursorModeActive() const;
        bool ShouldSuppressGamepadInput() const;
        bool IsMenuAvailable() const;
        void SetMenuAvailable(bool a_available);
        void SetCursorModeActive(bool a_active);

    private:
        void Run(std::stop_token a_stopToken);
        static bool IsGameWindowForeground();
        static void SetSkyrimCursorActive(bool a_active);
        static float ApplyDeadzone(float a_value, float a_deadzone);
        void QueueMouseMove(LONG a_dx, LONG a_dy);
        static void QueueMouseWheel(std::int32_t a_notches);
        static void QueueMouseButton(std::uint32_t a_button, bool a_down);
        static void QueueKeyboardButton(std::uint32_t a_key, bool a_down);

        std::jthread worker_;
        std::atomic_bool active_{ false };
        std::atomic_bool suppressGamepadInput_{ false };
        std::atomic_bool menuAvailable_{ false };
        std::atomic_bool moveTaskPending_{ false };
        std::atomic_long pendingX_{ 0 };
        std::atomic_long pendingY_{ 0 };
    };
}
