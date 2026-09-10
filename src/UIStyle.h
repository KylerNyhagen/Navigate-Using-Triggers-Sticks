#pragma once

#include "SKSEMenuFramework.h"

namespace GamepadCursorMode::UI::Style
{
	namespace ImGui = ImGuiMCP;

	inline constexpr ImGui::ImVec4 kOpaqueBg{ 0.10f, 0.10f, 0.12f, 1.0f };


	inline constexpr ImGui::ImVec4 kBodyText{ 0.88f, 0.88f, 0.90f, 1.0f };
	inline constexpr ImGui::ImVec4 kMutedText{ 0.66f, 0.68f, 0.74f, 1.0f };

	inline constexpr ImGui::ImVec4 kAccentText{ 0.55f, 0.78f, 1.00f, 1.0f };
	inline constexpr ImGui::ImVec4 kWarnText{ 0.95f, 0.78f, 0.35f, 1.0f };

	inline constexpr ImGui::ImVec4 kValidText{ 0.40f, 0.85f, 0.40f, 1.0f };
	inline constexpr ImGui::ImVec4 kInvalidText{ 0.90f, 0.35f, 0.35f, 1.0f };

	inline void Push()
	{
		ImGui::PushStyleColor(ImGui::ImGuiCol_PopupBg, kOpaqueBg);
		ImGui::PushStyleColor(ImGui::ImGuiCol_ChildBg, kOpaqueBg);
		ImGui::PushStyleColor(ImGui::ImGuiCol_Text, kBodyText);
		ImGui::PushStyleColor(ImGui::ImGuiCol_TextDisabled, kMutedText);
	}

	inline void Pop()
	{
		ImGui::PopStyleColor(4);
	}

	inline void Hint(const char* a_text)
	{
		ImGui::PushStyleColor(ImGui::ImGuiCol_Text, kMutedText);
		ImGui::TextWrapped("%s", a_text);
		ImGui::PopStyleColor();
	}

	inline void WarnHint(const char* a_text)
	{
		ImGui::PushStyleColor(ImGui::ImGuiCol_Text, kWarnText);
		ImGui::TextWrapped("%s", a_text);
		ImGui::PopStyleColor();
	}

	inline void Heading(const char* a_text)
	{
		ImGui::PushStyleColor(ImGui::ImGuiCol_Text, kAccentText);
		ImGui::SeparatorText(a_text);
		ImGui::PopStyleColor();
	}
}
