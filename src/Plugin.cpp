#include "Config.h"
#include "CursorController.h"
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
    InitializeLogging();
    SKSE::Init(a_skse);
    SKSE::log::info("Gamepad Cursor Mode loading");
    const auto* messaging = SKSE::GetMessagingInterface();
    return messaging && messaging->RegisterListener(OnSKSEMessage);
}

SKSEPluginVersion = []() noexcept {
    SKSE::PluginVersionData version{};
    version.PluginVersion({ GAMEPAD_CURSOR_MODE_VERSION_MAJOR, GAMEPAD_CURSOR_MODE_VERSION_MINOR, GAMEPAD_CURSOR_MODE_VERSION_PATCH });
    version.PluginName("Gamepad Cursor Mode");
    version.AuthorName("Kyler Nyhagen");
    version.UsesAddressLibrary();
    version.UsesUpdatedStructs();
    return version;
}();
