#include "NavElemnts.h"
#include <unordered_map>
//#include "../../cheat/menu/popups/pop.h"
#include <string>
#include "../../render/render.h"
using namespace ImGui;

struct tab_element
{
	float element_opacity, rect_opacity, text_opacity;
};

struct checkbox_animation
{
	float animation;
};

struct checkp_state
{
	ImVec4 background;
};

void RenderTextColor(ImFont* font, const ImVec2& p_min, const ImVec2& p_max, ImU32 col, const char* text, const ImVec2& align)
{
	PushFont(font);
	PushStyleColor(ImGuiCol_Text, col);
	RenderTextClipped(p_min, p_max, text, NULL, NULL, align, NULL);
	PopStyleColor();
	PopFont();
}

bool elements::checkboxP(const char* label, bool* v, const ImVec2& size_arg)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems) return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);
	const ImVec2 pos = window->DC.CursorPos;
	ImVec2 size = CalcItemSize(size_arg, label_size.x, label_size.y);
	const ImRect bb(pos, pos + size);

	ItemSize(size, 0.f);
	if (!ItemAdd(bb, id)) return false;

	bool hovered, held, pressed = ButtonBehavior(bb, id, &hovered, &held, NULL);
	if (IsItemClicked())
	{
		*v = !(*v);
		MarkItemEdited(id);
	}

	static std::map<ImGuiID, checkp_state> anim;
	checkp_state& state = anim[id];

	// Анимируем фон (но всегда стремится к черному)
	state.background = ImLerp(
		state.background,
		ImVec4(15 / 255.f, 15 / 255.f, 16 / 255.f, 1.0f), // всегда черный фон
		g.IO.DeltaTime * 6.f
	);

	// Заливаем фон
	ImGui::GetWindowDrawList()->AddRectFilled(bb.Min, bb.Max, ImGui::GetColorU32(state.background), 3.f);

	// Если кнопка активна - рисуем обводку фиолетовую
	if (*v)
	{
		ImVec4 activeBorderColor = ImVec4(196 / 255.f, 102 / 255.f, 255 / 255.f, 1.0f);
		ImGui::GetWindowDrawList()->AddRect(bb.Min, bb.Max, ImGui::GetColorU32(activeBorderColor), 2.f);
	}
	// Иначе обводку не рисуем вообще

	// Рисуем текст
	RenderTextColor(g_fonts->main, bb.Min, bb.Max - ImVec2(0, 2), ImGui::GetColorU32(ImVec4(1.f, 1.f, 1.f, 1.0f)), label, ImVec2(0.5, 0.5));

	return pressed;
}



bool elements::ClrEdit4(const char* label, c_color* color)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGui::AlignTextToFramePadding();
	ImGui::TextUnformatted(label);

	// Сдвигаем цветовой квадратик вправо, чтобы выровнять его с другими
	ImGui::SameLine(303); // <-- подбери значение под свой UI, можно чуть подвинуть

	return ImGui::ColorEdit4(
		("##" + std::string(label)).c_str(),
		(float*)color,
		ImGuiColorEditFlags_NoInputs |
		ImGuiColorEditFlags_NoTooltip |
		ImGuiColorEditFlags_NoLabel |
		ImGuiColorEditFlags_AlphaBar
	);
}

bool elements::PopupCheckbox(const char* label, bool* v, bool* penis)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

	const float w = ImGui::GetWindowWidth() - 30;
	const float square_sz = 17;
	const ImVec2 pos = window->DC.CursorPos;

	const float custom_padding = 3.0f;

	const ImRect frame_bb(
		pos + ImVec2(w - square_sz - 13, custom_padding),
		pos + ImVec2(w, square_sz + custom_padding - 1)
	);
	const ImRect total_bb(
		pos,
		pos + ImVec2(
			square_sz + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f),
			label_size.y + style.FramePadding.y * 2.0f + custom_padding * 2.0f
		)
	);

	ItemSize(total_bb, style.FramePadding.y);
	if (!ItemAdd(total_bb, id))
		return false;

	bool hovered, held;
	bool pressed = ButtonBehavior(frame_bb, id, &hovered, &held);
	if (pressed)
	{
		*v = !(*v);
		MarkItemEdited(id);
	}

	static std::map<ImGuiID, checkbox_animation> anim;
	auto& anim_val = anim[id].animation;
	anim_val = ImLerp(anim_val, *v ? 1.0f : 0.0f, 0.12f * (1.0f - ImGui::GetIO().DeltaTime));

	RenderNavHighlight(total_bb, id);

	RenderFrame(frame_bb.Min, frame_bb.Max, ImColor(15, 15, 16), false, 9.0f);
	RenderFrame(frame_bb.Min, frame_bb.Max, ImColor(196, 102, 255, static_cast<int>(anim_val * 255)), false, 9.0f);

	window->DrawList->AddCircleFilled(
		ImVec2(frame_bb.Min.x + 8 + 14 * anim_val, frame_bb.Min.y + 8),
		5.0f,
		IM_COL32_WHITE,
		30
	);

	// Кнопка-шестеренка
	ImVec2 gear_icon_size = ImVec2(20.0f, 20.0f); // Сделал чуть больше, 17x17
	ImVec2 gear_icon_pos = ImVec2(
		frame_bb.Max.x - square_sz - gear_icon_size.x - 15.0f, // Подвинул левее к тогглу
		frame_bb.Min.y + (frame_bb.GetHeight() - gear_icon_size.y) * 0.3f // Выровнял по центру
	);

	ImGui::SetCursorScreenPos(gear_icon_pos);
	ImGui::PushID(label);

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

	if (ImGui::Button("##gearbutton", gear_icon_size))
	{
		if (penis)
			*penis = true;
	}

	// Теперь рисуем шестерёнку по центру
	ImVec2 icon_pos = ImGui::GetItemRectMin();
	float gear_font_size = 15.0f; // Сделал шрифт побольше
	ImVec2 icon_size = ImGui::CalcTextSize("\xef\x80\x93", NULL, false, gear_font_size);

	ImVec2 text_pos = ImVec2(
		icon_pos.x + (gear_icon_size.x - icon_size.x) * 0.5f,
		icon_pos.y + (gear_icon_size.y - icon_size.y) * 0.5f
	);

	ImGui::GetWindowDrawList()->AddText(
		g_fonts->fasolid,
		gear_font_size,
		text_pos,
		IM_COL32_WHITE,
		"\xef\x80\x93"
	);

	ImGui::PopStyleColor(3);
	ImGui::PopID();

	if (label_size.x > 0.0f)
		RenderText(ImVec2(total_bb.Min.x, total_bb.Min.y + custom_padding), label);

	IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
	return pressed;
}

bool elements::ColorCheckbox(const char* label, bool* v, c_color* color)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

	const float w = ImGui::GetWindowWidth() - 30;
	const float square_sz = 17;
	const ImVec2 pos = window->DC.CursorPos;

	const float custom_padding = 3.0f; // Кастомный вертикальный паддинг

	// Увеличиваем высоту total_bb за счет паддинга
	const ImRect frame_bb(
		pos + ImVec2(w - square_sz - 13, custom_padding),
		pos + ImVec2(w, square_sz + custom_padding)
	);
	const ImRect total_bb(
		pos,
		pos + ImVec2(square_sz + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f),
			square_sz + custom_padding * 2.0f) // высота увеличена
	);

	ItemSize(total_bb, style.FramePadding.y);
	if (!ItemAdd(total_bb, id))
		return false;

	bool hovered, held;
	bool pressed = ButtonBehavior(frame_bb, id, &hovered, &held);
	if (pressed)
	{
		*v = !(*v);
		MarkItemEdited(id);
	}

	static std::map<ImGuiID, checkbox_animation> anim;
	auto& anim_val = anim[id].animation;
	anim_val = ImLerp(anim_val, *v ? 1.0f : 0.0f, 0.12f * (1.0f - ImGui::GetIO().DeltaTime));

	RenderNavHighlight(total_bb, id);

	RenderFrame(frame_bb.Min, frame_bb.Max, ImColor(15, 15, 16), false, 9.0f);
	RenderFrame(frame_bb.Min, frame_bb.Max, ImColor(196, 102, 255, static_cast<int>(anim_val * 255)), false, 9.0f);

	window->DrawList->AddCircleFilled(
		ImVec2(frame_bb.Min.x + 8 + 14 * anim_val, frame_bb.Min.y + 8),
		5.0f,
		IM_COL32_WHITE,
		30
	);

	// Отступы для цветовой кнопки
	constexpr float spacing_between_color_and_toggle = 10.0f;
	constexpr float left_offset = 10.0f;

	ImVec2 color_box_size = ImVec2(13.0f, 13.05f);
	ImVec2 color_box_pos = ImVec2(
		frame_bb.Min.x - color_box_size.x - spacing_between_color_and_toggle,
		frame_bb.Min.y + (frame_bb.GetHeight() - color_box_size.y) * 0.5f
	);
	color_box_pos.x -= left_offset;

	ImGui::SetCursorScreenPos(color_box_pos);
	ImGui::PushItemWidth(color_box_size.x);
	ImGui::ColorEdit4(("##" + std::string(label)).c_str(), (float*)color,
		ImGuiColorEditFlags_NoInputs |
		ImGuiColorEditFlags_NoTooltip |
		ImGuiColorEditFlags_NoLabel |
		ImGuiColorEditFlags_AlphaBar);
	ImGui::PopItemWidth();

	if (label_size.x > 0.0f)
		RenderText(total_bb.Min, label);

	IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
	return pressed;
}


bool elements::Checkbox(const char* label, bool* v)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

	const float w = ImGui::GetWindowWidth() - 30;
	const float square_sz = 17;
	const float custom_padding = 2.0f; // Кастомный вертикальный паддинг
	const ImVec2 pos = window->DC.CursorPos;

	const ImRect frame_bb(
		pos + ImVec2(w - square_sz - 13, custom_padding),
		pos + ImVec2(w, square_sz + custom_padding)
	);

	const ImRect total_bb(
		pos,
		pos + ImVec2(square_sz + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f),
			square_sz + custom_padding * 2.0f)
	);

	ItemSize(total_bb, style.FramePadding.y);
	if (!ItemAdd(total_bb, id))
	{
		IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
		return false;
	}

	bool hovered, held;
	bool pressed = ButtonBehavior(frame_bb, id, &hovered, &held);
	if (pressed)
	{
		*v = !(*v);
		MarkItemEdited(id);
	}

	static std::map<ImGuiID, checkbox_animation> anim;
	auto& anim_val = anim[id].animation;
	anim_val = ImLerp(anim_val, *v ? 1.0f : 0.0f, 0.12f * (1.0f - ImGui::GetIO().DeltaTime));

	RenderNavHighlight(total_bb, id);

	RenderFrame(frame_bb.Min, frame_bb.Max, ImColor(15, 15, 16), false, 9.0f);

	RenderFrame(
		frame_bb.Min,
		frame_bb.Max,
		ImColor(196 / 255.0f, 102 / 255.0f, 255 / 255.0f, anim_val),
		false,
		9.0f
	);

	window->DrawList->AddCircleFilled(
		ImVec2(frame_bb.Min.x + 8 + 14 * anim_val, frame_bb.Min.y + 8),
		5.0f,
		IM_COL32_WHITE,
		30
	);

	if (label_size.x > 0.0f)
		RenderText(total_bb.Min, label);

	IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
	return pressed;
}


bool elements::tab(const char* name, bool boolean)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(name);
	ImVec2 pos = window->DC.CursorPos;

	const ImRect rect(pos, ImVec2(pos.x + 31, pos.y + 31));
	ImGui::ItemSize(ImVec4(rect.Min.x, rect.Min.y, rect.Max.x, rect.Max.y + 5), style.FramePadding.y);
	if (!ImGui::ItemAdd(rect, id))
		return false;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(rect, id, &hovered, &held, NULL);

	static std::map<ImGuiID, tab_element> anim;
	auto it_anim = anim.find(id);
	if (it_anim == anim.end())
	{
		anim.insert({ id, { 0.0f, 0.0f, 0.0f } });
		it_anim = anim.find(id);
	}

	it_anim->second.element_opacity = ImLerp(it_anim->second.element_opacity, (boolean ? 0.04f : hovered ? 0.01f :
		0.0f),
		0.07f * (1.0f - ImGui::GetIO().DeltaTime));
	it_anim->second.rect_opacity = ImLerp(it_anim->second.rect_opacity, (boolean ? 1.0f : 0.0f), 0.15f * (1.0f - ImGui::GetIO().DeltaTime));
	it_anim->second.text_opacity = ImLerp(it_anim->second.text_opacity, (boolean ? 1.0f : hovered ? 0.5f :
		0.3f),
		0.07f * (1.0f - ImGui::GetIO().DeltaTime));

	window->DrawList->AddRectFilled(rect.Min, rect.Max, ImColor(1.0f, 1.0f, 1.0f, it_anim->second.element_opacity), 3.0f);

	window->DrawList->AddRectFilled(ImVec2(rect.Max.x + 4, rect.Min.y + 6), ImVec2(rect.Max.x + 8, rect.Max.y - 6), ImColor(147 / 255.0f, 190 / 255.0f, 66 / 255.0f, it_anim->second.rect_opacity), 7.0f, ImDrawFlags_RoundCornersLeft);

	return pressed;
}

struct subtab_element
{
	float element_opacity, rect_opacity, text_opacity;
};

bool elements::subtab(const char* name, bool boolean)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(name);
	const ImVec2 label_size = ImGui::CalcTextSize(name);
	ImVec2 pos = window->DC.CursorPos;

	const ImRect rect(pos, ImVec2(pos.x + 145, pos.y + 32));
	ImGui::ItemSize(ImVec4(rect.Min.x, rect.Min.y, rect.Max.x, rect.Max.y + 3), style.FramePadding.y);
	if (!ImGui::ItemAdd(rect, id))
		return false;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(rect, id, &hovered, &held, NULL);

	static std::map<ImGuiID, subtab_element> anim;
	auto it_anim = anim.find(id);
	if (it_anim == anim.end())
	{
		anim.insert({ id, { 0.0f, 0.0f, 0.0f } });
		it_anim = anim.find(id);
	}

	it_anim->second.element_opacity = ImLerp(it_anim->second.element_opacity, (boolean ? 0.04f : 0.0f), 0.09f * (1.0f - ImGui::GetIO().DeltaTime));
	it_anim->second.rect_opacity = ImLerp(it_anim->second.rect_opacity, (boolean ? 1.0f : 0.0f), 0.20f * (1.0f - ImGui::GetIO().DeltaTime));
	it_anim->second.text_opacity = ImLerp(it_anim->second.text_opacity, (boolean ? 1.0f : 0.3f), 0.07f * (1.0f - ImGui::GetIO().DeltaTime));

	window->DrawList->AddRectFilled(rect.Min, rect.Max, ImColor(1.0f, 1.0f, 1.0f, it_anim->second.element_opacity), 3.0f);
	window->DrawList->AddText(ImVec2(rect.Min.x + 15, (rect.Min.y + rect.Max.y) / 2 - label_size.y / 2), ImColor(1.0f, 1.0f, 1.0f, it_anim->second.text_opacity), name);

	window->DrawList->AddRectFilled(ImVec2(rect.Max.x + 5, rect.Min.y + 9), ImVec2(rect.Max.x + 8, rect.Max.y - 9), ImColor(147 / 255.0f, 190 / 255.0f, 66 / 255.0f, it_anim->second.rect_opacity), 10.0f, ImDrawFlags_RoundCornersLeft);

	return pressed;
}
