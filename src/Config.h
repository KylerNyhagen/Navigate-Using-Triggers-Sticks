#pragma once

namespace GamepadCursorMode
{
    enum class Stick : std::uint8_t { kLeft, kRight };

    struct Settings
    {
        bool enabled{ true };
        std::uint16_t toggleButton{ XINPUT_GAMEPAD_DPAD_RIGHT };
        Stick cursorStick{ Stick::kRight };
        float cursorSpeed{ 1100.0F };
        float deadzone{ 0.18F };
        bool leftTriggerClicks{ true };
        bool rightTriggerClicks{ true };
    };

    class Config final
    {
    public:
        static Config& GetSingleton();
        Settings Get() const;
        void Update(const Settings& a_settings);
        void Load();
        void Save() const;

    private:
        static std::filesystem::path Path();
        mutable std::mutex lock_;
        Settings settings_;
    };
}
