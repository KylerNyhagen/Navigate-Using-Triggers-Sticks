#include "Config.h"
#include "CursorController.h"
#include "InputHook.h"
#include "UI.h"
#include "Version.h"

namespace
{
    void InitializeLogging()
    {
        auto path = SKSE::log::log_directory();
        if (!path) return;
        *path /= "GamepadCursorMode.log";
        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
        auto logger = std::make_shared<spdlog::logger>("global", std::move(sink));
        logger->set_level(spdlog::level::info);
        logger->flush_on(spdlog::level::info);
        spdlog::set_default_logger(std::move(logger));
        spdlog::set_pattern("[%H:%M:%S] [%l] %v"s);
    }

    void OnSKSEMessage(SKSE::MessagingInterface::Message* a_message)
    {
        if (!a_message) return;
        if (a_message->type == SKSE::MessagingInterface::kPostLoad) {
            GamepadCursorMode::UI::Register();
        } else if (a_message->type == SKSE::MessagingInterface::kInputLoaded) {
            GamepadCursorMode::Config::GetSingleton().Load();
            GamepadCursorMode::CursorController::GetSingleton().Start();
        }
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
    // The plugin owns logging. Disable CommonLib's automatic second log file.
    // For some reason this kept making an empty log file for me with a different name. I want to go back and learn this a bit more in the future.
    SKSE::Init(a_skse, false);
    InitializeLogging();
    SKSE::log::info("Navigate Using Triggers & Sticks loading");
    GamepadCursorMode::InputHook::Install();
    const auto* messaging = SKSE::GetMessagingInterface();
    return messaging && messaging->RegisterListener(OnSKSEMessage);
}

SKSEPluginVersion = []() noexcept {
    SKSE::PluginVersionData version{};
    version.PluginVersion({ GAMEPAD_CURSOR_MODE_VERSION_MAJOR, GAMEPAD_CURSOR_MODE_VERSION_MINOR, GAMEPAD_CURSOR_MODE_VERSION_PATCH });
    version.PluginName("Navigate Using Triggers & Sticks");
    version.AuthorName("Kyler Nyhagen");
    version.UsesAddressLibrary();
    version.UsesUpdatedStructs();
    return version;
}();
