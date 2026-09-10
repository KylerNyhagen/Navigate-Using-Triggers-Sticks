#include "UI.h"
#include "Config.h"
#include "CursorController.h"
#include "SKSEMenuFramework.h"
#include "UIStyle.h"

namespace ImGui = ImGuiMCP;

namespace GamepadCursorMode::UI
{
    namespace
    {
        constexpr std::array<const char*, 14> kButtonNames{
            "D-pad Up", "D-pad Down", "D-pad Left", "D-pad Right", "Start", "Back",
            "Left Stick", "Right Stick", "Left Bumper", "Right Bumper", "A", "B", "X", "Y"
        };
        constexpr std::array<std::uint16_t, 14> kButtonMasks{
            XINPUT_GAMEPAD_DPAD_UP, XINPUT_GAMEPAD_DPAD_DOWN, XINPUT_GAMEPAD_DPAD_LEFT, XINPUT_GAMEPAD_DPAD_RIGHT,
            XINPUT_GAMEPAD_START, XINPUT_GAMEPAD_BACK, XINPUT_GAMEPAD_LEFT_THUMB, XINPUT_GAMEPAD_RIGHT_THUMB,
            XINPUT_GAMEPAD_LEFT_SHOULDER, XINPUT_GAMEPAD_RIGHT_SHOULDER, XINPUT_GAMEPAD_A, XINPUT_GAMEPAD_B,
            XINPUT_GAMEPAD_X, XINPUT_GAMEPAD_Y
        };

        int ButtonIndex(std::uint16_t a_mask)
        {
            const auto found = std::find(kButtonMasks.begin(), kButtonMasks.end(), a_mask);
            return found == kButtonMasks.end() ? 3 : static_cast<int>(std::distance(kButtonMasks.begin(), found));
        }

        void __stdcall Render()
        {
            UI::Style::Push();
            auto settings = Config::GetSingleton().Get();
            bool changed = false;

            ImGui::TextWrapped("Use a gamepad stick as a mouse cursor. The triggers can send left-click and right-click while cursor mode is active.");
            ImGui::Spacing();
            changed |= ImGui::Checkbox("Enable Navigate Using Triggers & Sticks", &settings.enabled);

            int button = ButtonIndex(settings.toggleButton);
            if (ImGui::Combo("Toggle Button", &button, kButtonNames.data(), static_cast<int>(kButtonNames.size()))) {
                settings.toggleButton = kButtonMasks[button];
                changed = true;
            }
            UI::Style::Hint("X is the default toggle. You can choose another button if your control setup uses X.");

            int stick = settings.cursorStick == Stick::kLeft ? 0 : 1;
            constexpr const char* sticks[]{ "Left Stick", "Right Stick" };
            if (ImGui::Combo("Cursor Stick", &stick, sticks, 2)) {
                settings.cursorStick = stick == 0 ? Stick::kLeft : Stick::kRight;
                changed = true;
            }
            UI::Style::Hint("The other stick sends mouse-wheel input.");
            changed |= ImGui::SliderFloat("Cursor Speed", &settings.cursorSpeed, 100.0F, 3000.0F, "%.0f pixels/second");
            changed |= ImGui::SliderFloat("Stick Deadzone", &settings.deadzone, 0.0F, 0.9F, "%.2f");
            changed |= ImGui::Checkbox("Left Trigger Sends Left-click", &settings.leftTriggerClicks);
            changed |= ImGui::Checkbox("Right Trigger Sends Right-click", &settings.rightTriggerClicks);

            ImGui::Spacing();
            ImGui::Text("Cursor mode is %s.", CursorController::GetSingleton().IsCursorModeActive() ? "active" : "inactive");

            if (changed) {
                Config::GetSingleton().Update(settings);
                Config::GetSingleton().Save();
            }
            UI::Style::Pop();
        }
    }

    void Register()
    {
        if (!SKSEMenuFramework::IsInstalled()) {
            SKSE::log::info("SKSE Menu Framework was not detected; settings remain available in the JSON file");
            return;
        }
        SKSEMenuFramework::SetSection("Navigate Using Triggers & Sticks");
        SKSEMenuFramework::AddSectionItem("Settings", Render);
        SKSE::log::info("registered the settings page with SKSE Menu Framework");
    }
}
