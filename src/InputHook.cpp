#include "InputHook.h"
#include "CursorController.h"

namespace GamepadCursorMode::InputHook
{
    namespace
    {
        using Dispatch_t = void(RE::BSTEventSource<RE::InputEvent*>*, RE::InputEvent* const*);
        REL::Relocation<Dispatch_t> original;

        void Dispatch(RE::BSTEventSource<RE::InputEvent*>* a_dispatcher, RE::InputEvent* const* a_events)
        {
            auto& controller = CursorController::GetSingleton();
            bool menuAvailable = false;
            if (auto* ui = RE::UI::GetSingleton()) {
                const auto cursorMenu = ui->GetMenu(RE::CursorMenu::MENU_NAME).get();
                for (const auto& menu : ui->menuStack) {
                    if (menu && menu.get() != cursorMenu && !menu->AlwaysOpen()) {
                        menuAvailable = true;
                        break;
                    }
                }
            }
            controller.SetMenuAvailable(menuAvailable);

            if (!a_events || !*a_events || !controller.ShouldSuppressGamepadInput()) {
                original(a_dispatcher, a_events);
                return;
            }

            auto* first = *a_events;
            RE::InputEvent* previous = nullptr;
            for (auto* current = first; current;) {
                auto* next = current->next;
                if (current->GetDevice() == RE::INPUT_DEVICE::kGamepad) {
                    if (previous) previous->next = next;
                    else first = next;
                } else {
                    previous = current;
                }
                current = next;
            }

            RE::InputEvent* filtered = first;
            original(a_dispatcher, &filtered);
        }
    }

    void Install()
    {
        SKSE::AllocTrampoline(14);
        auto& trampoline = SKSE::GetTrampoline();
        const auto address = REL::RelocationID(67315, 68617).address() + 0x7B;
        if (*reinterpret_cast<const std::uint8_t*>(address) != 0xE8) {
            SKSE::log::critical("input dispatch hook was not installed: expected a call at 1.6.1170 offset 0x7B");
            return;
        }
        original = trampoline.write_call<5>(address, Dispatch);
        SKSE::log::info("input dispatch hook installed");
    }
}
