#include "Config.h"

namespace GamepadCursorMode
{
    Config& Config::GetSingleton()
    {
        static Config instance;
        return instance;
    }

    Settings Config::Get() const
    {
        std::scoped_lock lock(lock_);
        return settings_;
    }

    void Config::Update(const Settings& a_settings)
    {
        std::scoped_lock lock(lock_);
        settings_ = a_settings;
    }

    std::filesystem::path Config::Path()
    {
        return std::filesystem::path("Data") / "SKSE" / "Plugins" / "GamepadCursorMode.json";
    }

    void Config::Load()
    {
        std::ifstream stream(Path());
        if (!stream) {
            SKSE::log::info("using default settings; config file was not found");
            return;
        }

        try {
            const auto json = nlohmann::json::parse(stream);
            auto next = Get();
            next.enabled = json.value("enabled", next.enabled);
            next.toggleButton = json.value("toggleButton", next.toggleButton);
            next.cursorStick = json.value("cursorStick", 1) == 0 ? Stick::kLeft : Stick::kRight;
            next.cursorSpeed = std::clamp(json.value("cursorSpeed", next.cursorSpeed), 100.0F, 3000.0F);
            next.deadzone = std::clamp(json.value("deadzone", next.deadzone), 0.0F, 0.9F);
            next.leftTriggerClicks = json.value("leftTriggerClicks", next.leftTriggerClicks);
            next.rightTriggerClicks = json.value("rightTriggerClicks", next.rightTriggerClicks);
            Update(next);
            SKSE::log::info("loaded settings");
        } catch (const std::exception& error) {
            SKSE::log::error("could not read settings: {}", error.what());
        }
    }

    void Config::Save() const
    {
        const auto settings = Get();
        const nlohmann::json json{
            { "enabled", settings.enabled },
            { "toggleButton", settings.toggleButton },
            { "cursorStick", settings.cursorStick == Stick::kLeft ? 0 : 1 },
            { "cursorSpeed", settings.cursorSpeed },
            { "deadzone", settings.deadzone },
            { "leftTriggerClicks", settings.leftTriggerClicks },
            { "rightTriggerClicks", settings.rightTriggerClicks }
        };

        try {
            std::filesystem::create_directories(Path().parent_path());
            std::ofstream stream(Path());
            stream << json.dump(2) << '\n';
        } catch (const std::exception& error) {
            SKSE::log::error("could not save settings: {}", error.what());
        }
    }
}
