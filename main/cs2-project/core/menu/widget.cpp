#include "widget.h"

using namespace ImGui;

float widgets::float_lerp(ImGuiID id, bool cond, float speed) {

    static std::map<ImGuiID, float> anim;
    auto value = anim.find(id);

    if (value == anim.end()) {
        anim.insert({ id, 0.f });
        value = anim.find(id);
    }

    value->second = ImLerp(value->second, (float)cond, speed);
    return value->second;
}

ImColor widgets::color_lerp(ImColor start, ImColor end, float stage)
{
    return ColorConvertFloat4ToU32(ImLerp(ImVec4(start.Value.x, start.Value.y, start.Value.z, start.Value.w), ImVec4(end.Value.x, end.Value.y, end.Value.z, end.Value.w), stage));
}

bool widgets::tab(const char* icon, const char* label, bool selected, float width)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    ImVec2 pos = window->DC.CursorPos;

    const ImRect bb(pos, pos + ImVec2(width, 40));
    ItemSize(bb, 0);
    if (!ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, NULL);

    float anim = float_lerp(id, selected);
    ImColor col = color_lerp(GetColorU32(ImGuiCol_TextDisabled), GetColorU32(ImGuiCol_Text), anim);
    ImColor col1 = color_lerp(GetColorU32(ImGuiCol_TextDisabled), GetColorU32(ImGuiCol_FrameBg), anim);

    window->DrawList->AddRectFilled(bb.Min, bb.Max, ImColor(21, 21, 23, (int)(anim * 255)), style.ChildRounding);
    window->DrawList->AddRectFilled(bb.Min + ImVec2(0, 10 + 10 * (1 - anim)),
        ImVec2(bb.Min.x + 3, bb.Max.y - 10 - 10 * (1 - anim)),
        GetColorU32(ImGuiCol_FrameBg, anim), 2.f, ImDrawFlags_RoundCornersRight);

    PushFont(GetIO().Fonts->Fonts[1]);

    // Новые отступы:
    const float icon_offset = 10.0f;  // Увеличенный отступ иконки от края (было 3)
    const float text_padding = 8.0f;  // Отступ текста от иконки

    // Размеры элементов
    ImVec2 icon_size = CalcTextSize(icon);
    ImVec2 label_size = CalcTextSize(label);

    // Позиция иконки (с увеличенным отступом)
    window->DrawList->AddText(
        ImVec2(bb.Min.x + icon_offset, bb.Min.y + (40 - icon_size.y) / 2),
        col1,
        icon
    );

    // Позиция текста (с отступом от иконки)
    window->DrawList->AddText(
        ImVec2(bb.Min.x + icon_offset + icon_size.x + text_padding,
            bb.Min.y + (40 - label_size.y) / 2),
        col,
        label
    );

    PopFont();

    return pressed;
}

void widgets::separator(const char* label)
{

    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    const ImVec2 pos = window->DC.CursorPos;

    const ImRect bb(pos, ImVec2(window->WorkRect.Max.x - 14, pos.y + 10));
    ItemSize(bb, 0);
    if (!ItemAdd(bb, 0))
        return;

    PushFont(GetIO().Fonts->Fonts[1]);

    ImVec2 label_size = CalcTextSize(label);
    window->DrawList->AddRectFilled(ImVec2(pos.x + label_size.x + 10, bb.Min.y + 4), ImVec2(bb.Max.x, bb.Max.y - 4), ImColor(21, 21, 23));
    window->DrawList->AddText(ImVec2(bb.Min.x, bb.Max.y - (bb.Max.y - bb.Min.y) / 2 - label_size.y / 2), GetColorU32(ImGuiCol_TextDisabled), label);

    PopFont();
}

bool widgets::subtab(const char* label, bool selected)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    ImVec2 pos = window->DC.CursorPos;

    const ImRect bb(pos, pos + ImVec2(100, 40));
    ItemSize(bb, 0);
    if (!ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, NULL);

    float anim = float_lerp(id, selected);
    ImColor col = color_lerp(GetColorU32(ImGuiCol_TextDisabled), GetColorU32(ImGuiCol_Text), anim);

    window->DrawList->AddRectFilled(bb.Min, bb.Max, ImColor(23, 23, 25, (int)(anim * 255)), style.ChildRounding);
    window->DrawList->AddRectFilled(ImVec2(bb.Min.x + 20 + 20 * (1 - anim), bb.Max.y - 3), bb.Max - ImVec2(20 + 20 * (1 - anim), 0), GetColorU32(ImGuiCol_FrameBg, anim), 2.f, ImDrawFlags_RoundCornersTop);

    PushFont(GetIO().Fonts->Fonts[1]);
    window->DrawList->AddText(bb.Max - (bb.Max - bb.Min) / 2 - CalcTextSize(label) / 2, col, label);
    PopFont();

    return pressed;
}
#include "../../render/render.h"
#include <unordered_map>
bool widgets::subtabsadd(const char* icon, bool selected)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(icon);

    ImVec2 pos = window->DC.CursorPos;

    const ImRect bb(pos, pos + ImVec2(75, 40));
    ItemSize(bb, 0);
    if (!ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, NULL);

    float anim = float_lerp(id, selected);
    ImColor col = color_lerp(GetColorU32(ImGuiCol_TextDisabled), GetColorU32(ImGuiCol_Text), anim);

    window->DrawList->AddRectFilled(bb.Min, bb.Max, ImColor(23, 23, 25, (int)(anim * 255)), style.ChildRounding);
    window->DrawList->AddRectFilled(ImVec2(bb.Min.x + 20 + 20 * (1 - anim), bb.Max.y - 3), bb.Max - ImVec2(20 + 20 * (1 - anim), 0), GetColorU32(ImGuiCol_FrameBg, anim), 2.f, ImDrawFlags_RoundCornersTop);

    window->DrawList->AddText(bb.Max - (bb.Max - bb.Min) / 2 - CalcTextSize(icon) / 2, col, icon);

    return pressed;
}

bool widgets::child(const char* icon, const char* label, const ImVec2& size_arg, ImGuiWindowFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* parent_window = g.CurrentWindow;
    ImGuiID id = parent_window->GetID(label);

    flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove;
    flags |= (parent_window->Flags & ImGuiWindowFlags_NoMove);

    const ImVec2 content_avail = GetContentRegionAvail();
    ImVec2 size = ImFloor(size_arg);
    const int auto_fit_axises = ((size.x == 0.0f) ? (1 << ImGuiAxis_X) : 0x00) | ((size.y == 0.0f) ? (1 << ImGuiAxis_Y) : 0x00);
    if (size.x <= 0.0f)
        size.x = ImMax(content_avail.x + size.x, 4.0f);

    const char* temp_window_name;
    if (label)
        ImFormatStringToTempBuffer(&temp_window_name, NULL, "%s/%s_%08X", parent_window->Name, label, id);
    else
        ImFormatStringToTempBuffer(&temp_window_name, NULL, "%s/%08X", parent_window->Name, id);

    const float backup_border_size = g.Style.ChildBorderSize;
    bool ret = Begin(temp_window_name, NULL, flags);
    g.Style.ChildBorderSize = backup_border_size;

    SetWindowSize(ImVec2(size.x, size_arg.y == 0 ? GetCurrentWindow()->ContentSize.y + GetStyle().WindowPadding.y * 2 : size.y));

    ImGuiWindow* child_window = g.CurrentWindow;
    child_window->ChildId = id;
    child_window->AutoFitChildAxises = (ImS8)auto_fit_axises;

    if (child_window->BeginCount == 1)
        parent_window->DC.CursorPos = child_window->Pos;

    const ImGuiID temp_id_for_activation = ImHashStr("##Child", 0, id);
    if (g.ActiveId == temp_id_for_activation)
        ClearActiveID();
    if (g.NavActivateId == id && !(flags & ImGuiWindowFlags_NavFlattened) && (child_window->DC.NavLayersActiveMask != 0 || child_window->DC.NavWindowHasScrollY))
    {
        FocusWindow(child_window);
        NavInitWindow(child_window, false);
        SetActiveID(temp_id_for_activation, child_window);
        g.ActiveIdSource = g.NavInputSource;
    }

    parent_window->DrawList->AddRectFilled(GetWindowPos() + ImVec2(0, 34), GetWindowPos() + ImVec2(size_arg.x, 35), GetColorU32(ImGuiCol_Border));
    ImVec2 Min = GetWindowPos();
    ImVec2 Max = GetWindowPos() + ImVec2(size_arg.x, 34);
    PushFont(GetIO().Fonts->Fonts[1]);
    parent_window->DrawList->AddText(Min + ImVec2(44, 34) - ((Min + ImVec2(44, 34)) - Min) / 2 - CalcTextSize(icon) / 2, GetColorU32(ImGuiCol_FrameBg), icon);
    parent_window->DrawList->AddText(ImVec2(Min.x + 44, Max.y - (Max.y - Min.y) / 2 - CalcTextSize(label).y / 2), GetColorU32(ImGuiCol_Text), label);
    PopFont();

    SetCursorPosY(GetCursorPosY() + 34);

    return ret;
}

bool widgets::checkbox(const char* label, bool* v)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    const float square_sz = GetFrameHeight();
    const ImVec2 pos = window->DC.CursorPos;
    const float w = GetContentRegionMax().x - style.WindowPadding.x;
    const ImRect bb(pos + ImVec2(w - square_sz, 0), pos + ImVec2(w, square_sz));
    ItemSize(bb, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);
    if (pressed)
    {
        *v = !(*v);
        MarkItemEdited(id);
    }

    float anim = float_lerp(id, *v);
    ImColor col = color_lerp(GetColorU32(ImGuiCol_TextDisabled), GetColorU32(ImGuiCol_Text), anim);

    window->DrawList->AddRectFilled(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg, anim), style.FrameRounding);
    window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_TextDisabled), style.FrameRounding);
    window->DrawList->AddText(bb.Max - (bb.Max - bb.Min) / 2 - CalcTextSize(ICON_FA_CHECK) / 2, GetColorU32(ImGuiCol_Text, anim), ICON_FA_CHECK);
    if (label_size.x > 0.0f)
        window->DrawList->AddText(ImVec2(pos.x, pos.y + style.FramePadding.y), col, label);

    return pressed;
}

bool widgets::popupcheckbox(const char* label, bool* v, bool* show_popup)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    const float square_sz = GetFrameHeight();
    const ImVec2 pos = window->DC.CursorPos;
    const float w = GetContentRegionMax().x - style.WindowPadding.x;

    // Основной чекбокс (справа)
    const ImRect checkbox_bb(pos + ImVec2(w - square_sz, 0), pos + ImVec2(w, square_sz));

    // Шестерёнка слева от чекбокса
    const float gear_size = square_sz * 0.9f;
    const float gear_padding = style.ItemInnerSpacing.x * 1.5f;
    const ImRect gear_bb(
        pos + ImVec2(w - square_sz - gear_size - gear_padding, 2.0f),
        pos + ImVec2(w - square_sz - gear_padding, 2.0f + gear_size)
    );

    // Область всего виджета
    const ImRect total_bb(pos, pos + ImVec2(w, square_sz));

    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id))
        return false;

    // Обработка кликов
    bool checkbox_hovered, checkbox_held;
    bool checkbox_pressed = ButtonBehavior(checkbox_bb, id, &checkbox_hovered, &checkbox_held);

    bool gear_active = *v; // Шестерёнка активна только при включенном чекбоксе
    bool gear_hovered = false, gear_held = false, gear_pressed = false;

    if (gear_active) {
        gear_pressed = ButtonBehavior(gear_bb, id + 1, &gear_hovered, &gear_held);
    }

    // Клик по чекбоксу
    bool value_changed = false;
    if (checkbox_pressed)
    {
        *v = !(*v);
        value_changed = true;
        MarkItemEdited(id);
    }

    // Клик по шестерёнке (только если чекбокс включен)
    if (gear_pressed && show_popup && gear_active)
        *show_popup = true;

    // Анимация (как в оригинале)
    float anim = float_lerp(id, *v);
    ImColor col = color_lerp(GetColorU32(ImGuiCol_TextDisabled), GetColorU32(ImGuiCol_Text), anim);

    // Рендер чекбокса (как в оригинале)
    window->DrawList->AddRectFilled(checkbox_bb.Min, checkbox_bb.Max, GetColorU32(ImGuiCol_FrameBg, anim), style.FrameRounding); // Фон с анимацией
    window->DrawList->AddRect(checkbox_bb.Min, checkbox_bb.Max, GetColorU32(ImGuiCol_TextDisabled), style.FrameRounding); // Обводка всегда видна

    // Галочка с анимацией
    window->DrawList->AddText(checkbox_bb.Max - (checkbox_bb.Max - checkbox_bb.Min) / 2 - CalcTextSize(ICON_FA_CHECK) / 2,
        GetColorU32(ImGuiCol_Text, anim), ICON_FA_CHECK);

    // Рендер шестерёнки
    if (g_fonts->fasolid)
    {
        ImGui::PushFont(g_fonts->fasolid);
        const ImVec2 text_size = CalcTextSize(ICON_FA_GEAR);
        const ImVec2 text_pos = gear_bb.Min + (gear_bb.GetSize() - text_size) * 0.5f;

        ImColor gear_color = color_lerp(
            GetColorU32(ImGuiCol_TextDisabled),
            GetColorU32(ImGuiCol_Text),
            anim
        ).Value;

        if (!*v) gear_color = GetColorU32(ImGuiCol_TextDisabled, 0.5f);

        window->DrawList->AddText(text_pos, gear_color, ICON_FA_GEAR);
        ImGui::PopFont();
    }

    // Рендер текста (как в оригинале)
    if (label_size.x > 0.0f)
        window->DrawList->AddText(ImVec2(pos.x, pos.y + style.FramePadding.y), col, label);

    return value_changed;
}
bool widgets::selectable(const char* label, bool selected, ImGuiSelectableFlags flags, const ImVec2& size_arg)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    ImGuiID id = window->GetID(label);
    ImVec2 label_size = CalcTextSize(label, NULL, true);
    ImVec2 size(size_arg.x != 0.0f ? size_arg.x : label_size.x, size_arg.y != 0.0f ? size_arg.y : label_size.y);
    ImVec2 pos = window->DC.CursorPos;
    pos.y += window->DC.CurrLineTextBaseOffset;
    ItemSize(size, 0.0f);

    const bool span_all_columns = (flags & ImGuiSelectableFlags_SpanAllColumns) != 0;
    const float min_x = span_all_columns ? window->ParentWorkRect.Min.x : pos.x;
    const float max_x = span_all_columns ? window->ParentWorkRect.Max.x : window->WorkRect.Max.x;
    if (size_arg.x == 0.0f || (flags & ImGuiSelectableFlags_SpanAvailWidth))
        size.x = ImMax(label_size.x, max_x - min_x);

    const ImVec2 text_min = pos;
    const ImVec2 text_max(min_x + size.x, pos.y + size.y);

    ImRect bb(min_x, pos.y, text_max.x, text_max.y);
    if ((flags & ImGuiSelectableFlags_NoPadWithHalfSpacing) == 0)
    {
        const float spacing_x = span_all_columns ? 0.0f : style.ItemSpacing.x;
        const float spacing_y = style.ItemSpacing.y;
        const float spacing_L = IM_FLOOR(spacing_x * 0.50f);
        const float spacing_U = IM_FLOOR(spacing_y * 0.50f);
        bb.Min.x -= spacing_L;
        bb.Min.y -= spacing_U;
        bb.Max.x += (spacing_x - spacing_L);
        bb.Max.y += (spacing_y - spacing_U);
    }

    const float backup_clip_rect_min_x = window->ClipRect.Min.x;
    const float backup_clip_rect_max_x = window->ClipRect.Max.x;
    if (span_all_columns)
    {
        window->ClipRect.Min.x = window->ParentWorkRect.Min.x;
        window->ClipRect.Max.x = window->ParentWorkRect.Max.x;
    }

    const bool disabled_item = (flags & ImGuiSelectableFlags_Disabled) != 0;
    const bool item_add = ItemAdd(bb, id, NULL, disabled_item ? ImGuiItemFlags_Disabled : ImGuiItemFlags_None);
    if (span_all_columns)
    {
        window->ClipRect.Min.x = backup_clip_rect_min_x;
        window->ClipRect.Max.x = backup_clip_rect_max_x;
    }

    if (!item_add)
        return false;

    const bool disabled_global = (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;
    if (disabled_item && !disabled_global)
        BeginDisabled();

    if (span_all_columns && window->DC.CurrentColumns)
        PushColumnsBackground();
    else if (span_all_columns && g.CurrentTable)
        TablePushBackgroundChannel();

    ImGuiButtonFlags button_flags = 0;
    if (flags & ImGuiSelectableFlags_NoHoldingActiveID) { button_flags |= ImGuiButtonFlags_NoHoldingActiveId; }
    if (flags & ImGuiSelectableFlags_NoSetKeyOwner) { button_flags |= ImGuiButtonFlags_NoSetKeyOwner; }
    if (flags & ImGuiSelectableFlags_SelectOnClick) { button_flags |= ImGuiButtonFlags_PressedOnClick; }
    if (flags & ImGuiSelectableFlags_SelectOnRelease) { button_flags |= ImGuiButtonFlags_PressedOnRelease; }
    if (flags & ImGuiSelectableFlags_AllowDoubleClick) { button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick; }
    if ((flags & ImGuiSelectableFlags_AllowOverlap) || (g.LastItemData.InFlags & ImGuiItemFlags_AllowOverlap)) { button_flags |= ImGuiButtonFlags_AllowOverlap; }

    const bool was_selected = selected;
    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, button_flags);

    if ((flags & ImGuiSelectableFlags_SelectOnNav) && g.NavJustMovedToId != 0 && g.NavJustMovedToFocusScopeId == g.CurrentFocusScopeId)
        if (g.NavJustMovedToId == id)
            selected = pressed = true;

    if (pressed || (hovered && (flags & ImGuiSelectableFlags_SetNavIdOnHover)))
    {
        if (!g.NavDisableMouseHover && g.NavWindow == window && g.NavLayer == window->DC.NavLayerCurrent)
        {
            SetNavID(id, window->DC.NavLayerCurrent, g.CurrentFocusScopeId, WindowRectAbsToRel(window, bb));
            g.NavDisableHighlight = true;
        }
    }
    if (pressed)
        MarkItemEdited(id);

    if (selected != was_selected)
        g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_ToggledSelection;

    if (hovered || selected)
    {
        const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
        RenderFrame(bb.Min, bb.Max, col, false, 0.0f);
    }
    if (g.NavId == id)
        RenderNavHighlight(bb, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);

    if (span_all_columns && window->DC.CurrentColumns)
        PopColumnsBackground();
    else if (span_all_columns && g.CurrentTable)
        TablePopBackgroundChannel();

    PushStyleColor(ImGuiCol_Text, GetColorU32(selected ? ImGuiCol_FrameBg : ImGuiCol_Text));
    RenderTextClipped(text_min, text_max, label, NULL, &label_size, style.SelectableTextAlign, &bb);
    PopStyleColor();

    if (pressed && (window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiSelectableFlags_DontClosePopups) && !(g.LastItemData.InFlags & ImGuiItemFlags_SelectableDontClosePopup))
        CloseCurrentPopup();

    if (disabled_item && !disabled_global)
        EndDisabled();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed;
}

bool widgets::selectable(const char* label, bool* p_selected, ImGuiSelectableFlags flags, const ImVec2& size_arg)
{
    if (widgets::selectable(label, *p_selected, flags, size_arg))
    {
        *p_selected = !*p_selected;
        return true;
    }

    return false;
}

static const char* Items_ArrayGetter(void* data, int idx)
{
    const char* const* items = (const char* const*)data;
    return items[idx];
}

static float CalcMaxPopupHeightFromItemCount(int items_count)
{
    ImGuiContext& g = *GImGui;
    if (items_count <= 0)
        return FLT_MAX;
    return (g.FontSize + g.Style.ItemSpacing.y) * items_count - g.Style.ItemSpacing.y + (g.Style.WindowPadding.y * 2);
}

bool widgets::begin_combo(const char* label, const char* preview_value, ImGuiComboFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();

    ImGuiNextWindowDataFlags backup_next_window_data_flags = g.NextWindowData.Flags;
    g.NextWindowData.ClearFlags();
    if (window->SkipItems)
        return false;

    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    IM_ASSERT((flags & (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)) != (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview));

    const float arrow_size = GetFrameHeight();
    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    const ImVec2 pos = window->DC.CursorPos;
    const float ww = GetContentRegionMax().x - style.WindowPadding.x;
    const ImRect bb(pos + ImVec2(ww - (ww / 2.0f + GetFrameHeight()), 0), pos + ImVec2(ww, label_size.y + style.FramePadding.y * 2.0f));
    ItemSize(bb, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);
    const ImGuiID popup_id = ImHashStr("##ComboPopup", 0, id);
    bool popup_open = IsPopupOpen(popup_id, ImGuiPopupFlags_None);
    if (pressed && !popup_open)
    {
        OpenPopupEx(popup_id, ImGuiPopupFlags_None);
        popup_open = true;
    }

    if (flags & ImGuiComboFlags_CustomPreview)
    {
        g.ComboPreviewData.PreviewRect = ImRect(bb.Min.x, bb.Min.y, bb.Max.x - arrow_size, bb.Max.y);
        IM_ASSERT(preview_value == NULL || preview_value[0] == 0);
        preview_value = NULL;
    }

    float anim = float_lerp(id, popup_open);
    ImColor col = color_lerp(GetColorU32(ImGuiCol_TextDisabled), GetColorU32(ImGuiCol_Text), anim);

    window->DrawList->AddRectFilled(bb.Min, bb.Max, GetColorU32(ImGuiCol_Border), style.FrameRounding);

    if (preview_value != NULL) {
        window->DrawList->AddText(ImVec2(bb.Max.x - arrow_size + style.FramePadding.y + 1, bb.Min.y + style.FramePadding.y - 1), GetColorU32(ImGuiCol_TextDisabled), ICON_FA_ARROW_UP_RIGHT_FROM_SQUARE);
        window->DrawList->AddText(bb.Min + style.FramePadding, GetColorU32(ImGuiCol_TextDisabled), preview_value);
    }
    if (label_size.x > 0)
        window->DrawList->AddText(ImVec2(pos.x, pos.y + style.FramePadding.y), col, label);

    if (!popup_open)
        return false;

    g.NextWindowData.Flags = backup_next_window_data_flags;
    if (!IsPopupOpen(popup_id, ImGuiPopupFlags_None))
    {
        g.NextWindowData.ClearFlags();
        return false;
    }

    float pw = bb.GetWidth();
    if (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSizeConstraint)
        g.NextWindowData.SizeConstraintRect.Min.x = ImMax(g.NextWindowData.SizeConstraintRect.Min.x, pw);
    else
    {
        if ((flags & ImGuiComboFlags_HeightMask_) == 0)
            flags |= ImGuiComboFlags_HeightSmall;
        IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiComboFlags_HeightMask_));
        int popup_max_height_in_items = -1;
        if (flags & ImGuiComboFlags_HeightRegular)     popup_max_height_in_items = 8;
        else if (flags & ImGuiComboFlags_HeightSmall)  popup_max_height_in_items = 6;
        else if (flags & ImGuiComboFlags_HeightLarge)  popup_max_height_in_items = 20;
        ImVec2 constraint_min(0.0f, 0.0f), constraint_max(FLT_MAX, FLT_MAX);
        if ((g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSize) == 0 || g.NextWindowData.SizeVal.x <= 0.0f)
            constraint_min.x = pw;
        if ((g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSize) == 0 || g.NextWindowData.SizeVal.y <= 0.0f)
            constraint_max.y = CalcMaxPopupHeightFromItemCount(popup_max_height_in_items) * anim;
        SetNextWindowSizeConstraints(constraint_min, constraint_max);
    }

    char name[16];
    ImFormatString(name, IM_ARRAYSIZE(name), "##Combo_%02d", g.BeginPopupStack.Size);

    if (ImGuiWindow* popup_window = FindWindowByName(name))
        if (popup_window->WasActive)
        {
            ImVec2 size_expected = CalcWindowNextAutoFitSize(popup_window);
            popup_window->AutoPosLastDirection = (flags & ImGuiComboFlags_PopupAlignLeft) ? ImGuiDir_Left : ImGuiDir_Down;
            ImRect r_outer = GetPopupAllowedExtentRect(popup_window);
            ImVec2 wpos = FindBestWindowPosForPopupEx(bb.GetBL(), size_expected, &popup_window->AutoPosLastDirection, r_outer, bb, ImGuiPopupPositionPolicy_ComboBox);
            SetNextWindowPos(wpos);
        }

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;// | ImGuiWindowFlags_NoScrollbar
    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
    bool ret = Begin(name, NULL, window_flags);
    PopStyleVar();
    if (!ret)
    {
        EndPopup();
        IM_ASSERT(0);
        return false;
    }
    return true;
}

bool widgets::combo(const char* label, int* current_item, const char* (*getter)(void* user_data, int idx), void* user_data, int items_count, int popup_max_height_in_items)
{
    ImGuiContext& g = *GImGui;

    const char* preview_value = NULL;
    if (*current_item >= 0 && *current_item < items_count)
        preview_value = getter(user_data, *current_item);

    if (popup_max_height_in_items != -1 && !(g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSizeConstraint))
        SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, CalcMaxPopupHeightFromItemCount(popup_max_height_in_items)));

    if (!widgets::begin_combo(label, preview_value, ImGuiComboFlags_None))
        return false;

    bool value_changed = false;
    for (int i = 0; i < items_count; i++)
    {
        const char* item_text = getter(user_data, i);
        if (item_text == NULL)
            item_text = "*Unknown item*";

        PushID(i);
        const bool item_selected = (i == *current_item);
        if (widgets::selectable(item_text, item_selected) && *current_item != i)
        {
            value_changed = true;
            *current_item = i;
        }
        if (item_selected)
            SetItemDefaultFocus();
        PopID();
    }

    EndCombo();

    if (value_changed)
        MarkItemEdited(g.LastItemData.ID);

    return value_changed;
}

bool widgets::combo(const char* label, int* current_item, const char* const items[], int items_count, int height_in_items)
{
    const bool value_changed = widgets::combo(label, current_item, Items_ArrayGetter, (void*)items, items_count, height_in_items);
    return value_changed;
}

static void ColorEditRestoreHS(const float* col, float* H, float* S, float* V)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.ColorEditCurrentID != 0);
    if (g.ColorEditSavedID != g.ColorEditCurrentID || g.ColorEditSavedColor != ColorConvertFloat4ToU32(ImVec4(col[0], col[1], col[2], 0)))
        return;

    if (*S == 0.0f || (*H == 0.0f && g.ColorEditSavedHue == 1))
        *H = g.ColorEditSavedHue;

    if (*V == 0.0f)
        *S = g.ColorEditSavedSat;
}

static void ColorEditRestoreH(const float* col, float* H)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.ColorEditCurrentID != 0);
    if (g.ColorEditSavedID != g.ColorEditCurrentID || g.ColorEditSavedColor != ColorConvertFloat4ToU32(ImVec4(col[0], col[1], col[2], 0)))
        return;
    *H = g.ColorEditSavedHue;
}

bool widgets::color_picker4(const char* label, float col[4], ImGuiColorEditFlags flags, const float* ref_col)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImDrawList* draw_list = window->DrawList;
    ImGuiStyle& style = g.Style;
    ImGuiIO& io = g.IO;

    const float width = CalcItemWidth();
    g.NextItemData.ClearFlags();

    PushID(label);
    const bool set_current_color_edit_id = (g.ColorEditCurrentID == 0);
    if (set_current_color_edit_id)
        g.ColorEditCurrentID = window->IDStack.back();
    BeginGroup();

    if (!(flags & ImGuiColorEditFlags_NoSidePreview))
        flags |= ImGuiColorEditFlags_NoSmallPreview;

    if (!(flags & ImGuiColorEditFlags_NoOptions))
        ColorPickerOptionsPopup(col, flags);

    if (!(flags & ImGuiColorEditFlags_PickerMask_))
        flags |= ((g.ColorEditOptions & ImGuiColorEditFlags_PickerMask_) ? g.ColorEditOptions : ImGuiColorEditFlags_DefaultOptions_) & ImGuiColorEditFlags_PickerMask_;
    if (!(flags & ImGuiColorEditFlags_InputMask_))
        flags |= ((g.ColorEditOptions & ImGuiColorEditFlags_InputMask_) ? g.ColorEditOptions : ImGuiColorEditFlags_DefaultOptions_) & ImGuiColorEditFlags_InputMask_;
    IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags_PickerMask_));
    IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags_InputMask_));
    if (!(flags & ImGuiColorEditFlags_NoOptions))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags_AlphaBar);

    int components = (flags & ImGuiColorEditFlags_NoAlpha) ? 3 : 4;
    bool alpha_bar = (flags & ImGuiColorEditFlags_AlphaBar) && !(flags & ImGuiColorEditFlags_NoAlpha);

    ImVec2 picker_pos = window->DC.CursorPos;

    float square_sz = GetFrameHeight();
    float bars_width = 8.0f;
    float sv_picker_size = ImMax(bars_width, width - (alpha_bar ? 2 : 1) * (bars_width + style.ItemInnerSpacing.x));
    float bar0_pos_x = picker_pos.x;
    float bar1_pos_x = picker_pos.x;

    float backup_initial_col[4];
    memcpy(backup_initial_col, col, components * sizeof(float));

    float wheel_thickness = sv_picker_size * 0.08f;
    float wheel_r_outer = sv_picker_size * 0.50f;
    float wheel_r_inner = wheel_r_outer - wheel_thickness;

    ImVec2 wheel_center(picker_pos.x + (sv_picker_size + bars_width) * 0.5f, picker_pos.y + sv_picker_size * 0.5f);

    float triangle_r = wheel_r_inner - (int)(sv_picker_size * 0.027f);

    ImVec2 triangle_pa = ImVec2(triangle_r, 0.0f);
    ImVec2 triangle_pb = ImVec2(triangle_r * -0.5f, triangle_r * -0.866025f);
    ImVec2 triangle_pc = ImVec2(triangle_r * -0.5f, triangle_r * +0.866025f);

    float H = col[0], S = col[1], V = col[2];
    float R = col[0], G = col[1], B = col[2];
    if (flags & ImGuiColorEditFlags_InputRGB)
    {
        ColorConvertRGBtoHSV(R, G, B, H, S, V);
        ColorEditRestoreHS(col, &H, &S, &V);
    }
    else if (flags & ImGuiColorEditFlags_InputHSV)
        ColorConvertHSVtoRGB(H, S, V, R, G, B);

    bool value_changed = false, value_changed_h = false, value_changed_sv = false;

    PushItemFlag(ImGuiItemFlags_NoNav, true);
    if (flags & ImGuiColorEditFlags_PickerHueWheel)
    {
        InvisibleButton("hsv", ImVec2(sv_picker_size + style.ItemInnerSpacing.x + bars_width, sv_picker_size));
        if (IsItemActive())
        {
            ImVec2 initial_off = g.IO.MouseClickedPos[0] - wheel_center;
            ImVec2 current_off = g.IO.MousePos - wheel_center;
            float initial_dist2 = ImLengthSqr(initial_off);
            if (initial_dist2 >= (wheel_r_inner - 1) * (wheel_r_inner - 1) && initial_dist2 <= (wheel_r_outer + 1) * (wheel_r_outer + 1))
            {
                H = ImAtan2(current_off.y, current_off.x) / IM_PI * 0.5f;
                if (H < 0.0f)
                    H += 1.0f;
                value_changed = value_changed_h = true;
            }

            float cos_hue_angle = ImCos(-H * 2.0f * IM_PI);
            float sin_hue_angle = ImSin(-H * 2.0f * IM_PI);
            if (ImTriangleContainsPoint(triangle_pa, triangle_pb, triangle_pc, ImRotate(initial_off, cos_hue_angle, sin_hue_angle)))
            {
                ImVec2 current_off_unrotated = ImRotate(current_off, cos_hue_angle, sin_hue_angle);
                if (!ImTriangleContainsPoint(triangle_pa, triangle_pb, triangle_pc, current_off_unrotated))
                    current_off_unrotated = ImTriangleClosestPoint(triangle_pa, triangle_pb, triangle_pc, current_off_unrotated);
                float uu, vv, ww;
                ImTriangleBarycentricCoords(triangle_pa, triangle_pb, triangle_pc, current_off_unrotated, uu, vv, ww);
                V = ImClamp(1.0f - vv, 0.0001f, 1.0f);
                S = ImClamp(uu / V, 0.0001f, 1.0f);
                value_changed = value_changed_sv = true;
            }
        }
        if (!(flags & ImGuiColorEditFlags_NoOptions))
            OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);
    }
    else if (flags & ImGuiColorEditFlags_PickerHueBar)
    {
        InvisibleButton("sv", ImVec2(sv_picker_size, sv_picker_size));
        if (IsItemActive())
        {
            S = ImSaturate((io.MousePos.x - picker_pos.x) / (sv_picker_size - 1));
            V = 1.0f - ImSaturate((io.MousePos.y - picker_pos.y) / (sv_picker_size - 1));
            ColorEditRestoreH(col, &H);
            value_changed = value_changed_sv = true;
        }
        if (!(flags & ImGuiColorEditFlags_NoOptions))
            OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);

        SetCursorScreenPos(ImVec2(bar0_pos_x, picker_pos.y + sv_picker_size + style.ItemInnerSpacing.y));
        InvisibleButton("hue", ImVec2(sv_picker_size, bars_width));
        if (IsItemActive())
        {
            H = ImSaturate((io.MousePos.x - picker_pos.x) / (sv_picker_size - 1));
            value_changed = value_changed_h = true;
        }
    }

    if (alpha_bar)
    {
        SetCursorScreenPos(ImVec2(bar1_pos_x, picker_pos.y + sv_picker_size + bars_width + style.ItemInnerSpacing.y * 2));
        InvisibleButton("alpha", ImVec2(sv_picker_size, bars_width));
        if (IsItemActive())
        {
            col[3] = 1.0f - ImSaturate((io.MousePos.x - picker_pos.x) / (sv_picker_size - 1));
            value_changed = true;
        }
    }
    PopItemFlag();

    if (!(flags & ImGuiColorEditFlags_NoSidePreview))
    {
        SameLine(0, style.ItemInnerSpacing.x);
        BeginGroup();
    }

    if (!(flags & ImGuiColorEditFlags_NoLabel))
    {
        const char* label_display_end = FindRenderedTextEnd(label);
        if (label != label_display_end)
        {
            if ((flags & ImGuiColorEditFlags_NoSidePreview))
                SameLine(0, style.ItemInnerSpacing.x);
            TextEx(label, label_display_end);
        }
    }

    if (!(flags & ImGuiColorEditFlags_NoSidePreview))
    {
        PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
        ImVec4 col_v4(col[0], col[1], col[2], (flags & ImGuiColorEditFlags_NoAlpha) ? 1.0f : col[3]);
        if ((flags & ImGuiColorEditFlags_NoLabel))
            Text("Current");

        ImGuiColorEditFlags sub_flags_to_forward = ImGuiColorEditFlags_InputMask_ | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_NoTooltip;
        ColorButton("##current", col_v4, (flags & sub_flags_to_forward), ImVec2(square_sz * 3, square_sz * 2));
        if (ref_col != NULL)
        {
            Text("Original");
            ImVec4 ref_col_v4(ref_col[0], ref_col[1], ref_col[2], (flags & ImGuiColorEditFlags_NoAlpha) ? 1.0f : ref_col[3]);
            if (ColorButton("##original", ref_col_v4, (flags & sub_flags_to_forward), ImVec2(square_sz * 3, square_sz * 2)))
            {
                memcpy(col, ref_col, components * sizeof(float));
                value_changed = true;
            }
        }
        PopItemFlag();
        EndGroup();
    }

    if (value_changed_h || value_changed_sv)
    {
        if (flags & ImGuiColorEditFlags_InputRGB)
        {
            ColorConvertHSVtoRGB(H, S, V, col[0], col[1], col[2]);
            g.ColorEditSavedHue = H;
            g.ColorEditSavedSat = S;
            g.ColorEditSavedID = g.ColorEditCurrentID;
            g.ColorEditSavedColor = ColorConvertFloat4ToU32(ImVec4(col[0], col[1], col[2], 0));
        }
        else if (flags & ImGuiColorEditFlags_InputHSV)
        {
            col[0] = H;
            col[1] = S;
            col[2] = V;
        }
    }

    bool value_changed_fix_hue_wrap = false;
    if ((flags & ImGuiColorEditFlags_NoInputs) == 0)
    {
        PushItemWidth((alpha_bar ? bar1_pos_x : bar0_pos_x) + bars_width - picker_pos.x);
        ImGuiColorEditFlags sub_flags_to_forward = ImGuiColorEditFlags_DataTypeMask_ | ImGuiColorEditFlags_InputMask_ | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaPreviewHalf;
        ImGuiColorEditFlags sub_flags = (flags & sub_flags_to_forward) | ImGuiColorEditFlags_NoPicker;
        if (flags & ImGuiColorEditFlags_DisplayRGB || (flags & ImGuiColorEditFlags_DisplayMask_) == 0)
            if (ColorEdit4("##rgb", col, sub_flags | ImGuiColorEditFlags_DisplayRGB))
            {
                value_changed_fix_hue_wrap = (g.ActiveId != 0 && !g.ActiveIdAllowOverlap);
                value_changed = true;
            }
        if (flags & ImGuiColorEditFlags_DisplayHSV || (flags & ImGuiColorEditFlags_DisplayMask_) == 0)
            value_changed |= ColorEdit4("##hsv", col, sub_flags | ImGuiColorEditFlags_DisplayHSV);
        if (flags & ImGuiColorEditFlags_DisplayHex || (flags & ImGuiColorEditFlags_DisplayMask_) == 0)
            value_changed |= ColorEdit4("##hex", col, sub_flags | ImGuiColorEditFlags_DisplayHex);
        PopItemWidth();
    }

    if (value_changed_fix_hue_wrap && (flags & ImGuiColorEditFlags_InputRGB))
    {
        float new_H, new_S, new_V;
        ColorConvertRGBtoHSV(col[0], col[1], col[2], new_H, new_S, new_V);
        if (new_H <= 0 && H > 0)
        {
            if (new_V <= 0 && V != new_V)
                ColorConvertHSVtoRGB(H, S, new_V <= 0 ? V * 0.5f : new_V, col[0], col[1], col[2]);
            else if (new_S <= 0)
                ColorConvertHSVtoRGB(H, new_S <= 0 ? S * 0.5f : new_S, new_V, col[0], col[1], col[2]);
        }
    }

    if (value_changed)
    {
        if (flags & ImGuiColorEditFlags_InputRGB)
        {
            R = col[0];
            G = col[1];
            B = col[2];
            ColorConvertRGBtoHSV(R, G, B, H, S, V);
            ColorEditRestoreHS(col, &H, &S, &V);
        }
        else if (flags & ImGuiColorEditFlags_InputHSV)
        {
            H = col[0];
            S = col[1];
            V = col[2];
            ColorConvertHSVtoRGB(H, S, V, R, G, B);
        }
    }

    const int style_alpha8 = IM_F32_TO_INT8_SAT(style.Alpha);

    const ImU32 col_black = IM_COL32(0, 0, 0, style_alpha8);
    const ImU32 col_white = IM_COL32(255, 255, 255, style_alpha8);
    const ImU32 col_midgrey = IM_COL32(128, 128, 128, style_alpha8);
    const ImU32 col_hues[6 + 1] = { IM_COL32(255,0,0,style_alpha8), IM_COL32(255,255,0,style_alpha8), IM_COL32(0,255,0,style_alpha8), IM_COL32(0,255,255,style_alpha8), IM_COL32(0,0,255,style_alpha8), IM_COL32(255,0,255,style_alpha8), IM_COL32(255,0,0,style_alpha8) };

    ImVec4 hue_color_f(1, 1, 1, style.Alpha); ColorConvertHSVtoRGB(H, 1, 1, hue_color_f.x, hue_color_f.y, hue_color_f.z);
    ImU32 hue_color32 = ColorConvertFloat4ToU32(hue_color_f);
    ImU32 user_col32_striped_of_alpha = ColorConvertFloat4ToU32(ImVec4(R, G, B, style.Alpha));

    ImVec2 sv_cursor_pos;

    if (flags & ImGuiColorEditFlags_PickerHueWheel)
    {
        const float aeps = 0.5f / wheel_r_outer;
        const int segment_per_arc = ImMax(4, (int)wheel_r_outer / 12);
        for (int n = 0; n < 6; n++)
        {
            const float a0 = (n) / 6.0f * 2.0f * IM_PI - aeps;
            const float a1 = (n + 1.0f) / 6.0f * 2.0f * IM_PI + aeps;
            const int vert_start_idx = draw_list->VtxBuffer.Size;
            draw_list->PathArcTo(wheel_center, (wheel_r_inner + wheel_r_outer) * 0.5f, a0, a1, segment_per_arc);
            draw_list->PathStroke(col_white, 0, wheel_thickness);
            const int vert_end_idx = draw_list->VtxBuffer.Size;

            ImVec2 gradient_p0(wheel_center.x + ImCos(a0) * wheel_r_inner, wheel_center.y + ImSin(a0) * wheel_r_inner);
            ImVec2 gradient_p1(wheel_center.x + ImCos(a1) * wheel_r_inner, wheel_center.y + ImSin(a1) * wheel_r_inner);
            ShadeVertsLinearColorGradientKeepAlpha(draw_list, vert_start_idx, vert_end_idx, gradient_p0, gradient_p1, col_hues[n], col_hues[n + 1]);
        }

        float cos_hue_angle = ImCos(H * 2.0f * IM_PI);
        float sin_hue_angle = ImSin(H * 2.0f * IM_PI);

        ImVec2 hue_cursor_pos(wheel_center.x + cos_hue_angle * (wheel_r_inner + wheel_r_outer) * 0.5f, wheel_center.y + sin_hue_angle * (wheel_r_inner + wheel_r_outer) * 0.5f);

        float hue_cursor_rad = value_changed_h ? wheel_thickness * 0.65f : wheel_thickness * 0.55f;
        int hue_cursor_segments = draw_list->_CalcCircleAutoSegmentCount(hue_cursor_rad);

        draw_list->AddCircleFilled(hue_cursor_pos, hue_cursor_rad, hue_color32, hue_cursor_segments);
        draw_list->AddCircle(hue_cursor_pos, hue_cursor_rad + 1, col_midgrey, hue_cursor_segments);
        draw_list->AddCircle(hue_cursor_pos, hue_cursor_rad, col_white, hue_cursor_segments);

        ImVec2 tra = wheel_center + ImRotate(triangle_pa, cos_hue_angle, sin_hue_angle);
        ImVec2 trb = wheel_center + ImRotate(triangle_pb, cos_hue_angle, sin_hue_angle);
        ImVec2 trc = wheel_center + ImRotate(triangle_pc, cos_hue_angle, sin_hue_angle);
        ImVec2 uv_white = GetFontTexUvWhitePixel();
        draw_list->PrimReserve(3, 3);
        draw_list->PrimVtx(tra, uv_white, hue_color32);
        draw_list->PrimVtx(trb, uv_white, col_black);
        draw_list->PrimVtx(trc, uv_white, col_white);
        draw_list->AddTriangle(tra, trb, trc, col_midgrey, 1.5f);
        sv_cursor_pos = ImLerp(ImLerp(trc, tra, ImSaturate(S)), trb, ImSaturate(1 - V));
    }
    else if (flags & ImGuiColorEditFlags_PickerHueBar)
    {
        draw_list->AddRectFilledMultiColorRounded1(picker_pos, picker_pos + ImVec2(sv_picker_size, sv_picker_size), GetColorU32(ImGuiCol_PopupBg), col_white, hue_color32, hue_color32, col_white, 4.0f);
        draw_list->AddRectFilledMultiColorRounded1(picker_pos, picker_pos + ImVec2(sv_picker_size, sv_picker_size), GetColorU32(ImGuiCol_PopupBg), 0, 0, col_black, col_black, 4.0f);
        RenderFrameBorder(picker_pos, picker_pos + ImVec2(sv_picker_size, sv_picker_size), 4.0f);
        sv_cursor_pos.x = ImClamp(IM_ROUND(picker_pos.x + ImSaturate(S) * sv_picker_size), picker_pos.x + 2, picker_pos.x + sv_picker_size - 2);
        sv_cursor_pos.y = ImClamp(IM_ROUND(picker_pos.y + ImSaturate(1 - V) * sv_picker_size), picker_pos.y + 2, picker_pos.y + sv_picker_size - 2);

        draw_list->AddRectFilledMultiColorRounded1(ImVec2(bar0_pos_x + 0 * (sv_picker_size / 6), picker_pos.y + sv_picker_size + style.ItemInnerSpacing.y), ImVec2(bar0_pos_x + (0 + 1) * (sv_picker_size / 6), picker_pos.y + sv_picker_size + style.ItemInnerSpacing.y + bars_width), GetColorU32(ImGuiCol_PopupBg), col_hues[0], col_hues[0 + 1], col_hues[0 + 1], col_hues[0], 4, ImDrawFlags_RoundCornersLeft | ImDrawFlags_RoundCornersBottomLeft);
        for (int i = 1; i < 5; ++i)
            draw_list->AddRectFilledMultiColor(ImVec2(bar0_pos_x + i * (sv_picker_size / 6), picker_pos.y + sv_picker_size + style.ItemInnerSpacing.y), ImVec2(bar0_pos_x + (i + 1) * (sv_picker_size / 6), picker_pos.y + sv_picker_size + style.ItemInnerSpacing.y + bars_width), col_hues[i], col_hues[i + 1], col_hues[i + 1], col_hues[i]);
        draw_list->AddRectFilledMultiColorRounded1(ImVec2(bar0_pos_x + 5 * (sv_picker_size / 6), picker_pos.y + sv_picker_size + style.ItemInnerSpacing.y), ImVec2(bar0_pos_x + (5 + 1) * (sv_picker_size / 6), picker_pos.y + sv_picker_size + style.ItemInnerSpacing.y + bars_width), GetColorU32(ImGuiCol_PopupBg), col_hues[5], col_hues[5 + 1], col_hues[5 + 1], col_hues[5], 4, ImDrawFlags_RoundCornersRight | ImDrawFlags_RoundCornersBottomRight);

        float bar0_line_x = IM_ROUND(picker_pos.x + H * sv_picker_size);
        GetWindowDrawList()->AddCircleFilled(ImVec2(bar0_line_x + 1, picker_pos.y + sv_picker_size + style.ItemInnerSpacing.y + 4), 6, hue_color32);
        GetWindowDrawList()->AddCircle(ImVec2(bar0_line_x + 1, picker_pos.y + sv_picker_size + style.ItemInnerSpacing.y + 4), 7, col_midgrey);
        GetWindowDrawList()->AddCircle(ImVec2(bar0_line_x + 1, picker_pos.y + sv_picker_size + style.ItemInnerSpacing.y + 4), 6, col_white);
    }

    float sv_cursor_rad = value_changed_sv ? 10.0f : 6.0f;
    int sv_cursor_segments = draw_list->_CalcCircleAutoSegmentCount(sv_cursor_rad);

    draw_list->AddCircleFilled(sv_cursor_pos, sv_cursor_rad, user_col32_striped_of_alpha, sv_cursor_segments);
    draw_list->AddCircle(sv_cursor_pos, sv_cursor_rad + 1, col_midgrey, sv_cursor_segments);
    draw_list->AddCircle(sv_cursor_pos, sv_cursor_rad, col_white, sv_cursor_segments);

    if (alpha_bar)
    {
        float alpha = ImSaturate(col[3]);
        ImRect bar1_bb(picker_pos + ImVec2(0, sv_picker_size + style.ItemInnerSpacing.y * 2 + bars_width), picker_pos + ImVec2(sv_picker_size, sv_picker_size + style.ItemInnerSpacing.y * 2 + bars_width * 2));

        draw_list->AddRectFilled(bar1_bb.Min, bar1_bb.Max, ImColor(0.f, 0.f, 0.f), 3);
        draw_list->AddRectFilledMultiColorRounded(bar1_bb.Min, bar1_bb.Max, GetColorU32(ImGuiCol_PopupBg), ImColor(col[0], col[1], col[2]), ImColor(col[0], col[1], col[2], 0.f), ImColor(col[0], col[1], col[2], 0.f), ImColor(col[0], col[1], col[2]), 3);

        float bar1_line_x = IM_ROUND(picker_pos.x + (1.0f - alpha) * sv_picker_size);

        GetWindowDrawList()->AddCircleFilled(ImVec2(bar1_line_x + 1, bar1_bb.GetCenter().y), 6, ImColor(col[0], col[1], col[2]));
        GetWindowDrawList()->AddCircle(ImVec2(bar1_line_x + 1, bar1_bb.GetCenter().y), 7, col_midgrey);
        GetWindowDrawList()->AddCircle(ImVec2(bar1_line_x + 1, bar1_bb.GetCenter().y), 6, col_white);
    }

    EndGroup();

    if (value_changed && memcmp(backup_initial_col, col, components * sizeof(float)) == 0)
        value_changed = false;
    if (value_changed && g.LastItemData.ID != 0)
        MarkItemEdited(g.LastItemData.ID);

    if (set_current_color_edit_id)
        g.ColorEditCurrentID = 0;
    PopID();

    return value_changed;
}

bool widgets::color_edit4(const char* label, float col[4], ImGuiColorEditFlags flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const float square_sz = GetFrameHeight();
    const float w_button = (flags & ImGuiColorEditFlags_NoSmallPreview) ? 0.0f : (square_sz + style.ItemInnerSpacing.x);
    const float w_inputs = CalcItemWidth() - w_button;
    const char* label_display_end = FindRenderedTextEnd(label);
    g.NextItemData.ClearFlags();

    BeginGroup();
    PushID(label);

    const bool set_current_color_edit_id = (g.ColorEditCurrentID == 0);
    if (set_current_color_edit_id)
        g.ColorEditCurrentID = window->IDStack.back();

    const ImGuiColorEditFlags flags_untouched = flags;
    if (flags & ImGuiColorEditFlags_NoInputs)
        flags = (flags & (~ImGuiColorEditFlags_DisplayMask_)) | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoOptions;

    if (!(flags & ImGuiColorEditFlags_NoOptions))
        ColorEditOptionsPopup(col, flags);

    if (!(flags & ImGuiColorEditFlags_DisplayMask_))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags_DisplayMask_);
    if (!(flags & ImGuiColorEditFlags_DataTypeMask_))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags_DataTypeMask_);
    if (!(flags & ImGuiColorEditFlags_PickerMask_))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags_PickerMask_);
    if (!(flags & ImGuiColorEditFlags_InputMask_))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags_InputMask_);
    flags |= (g.ColorEditOptions & ~(ImGuiColorEditFlags_DisplayMask_ | ImGuiColorEditFlags_DataTypeMask_ | ImGuiColorEditFlags_PickerMask_ | ImGuiColorEditFlags_InputMask_));
    IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags_DisplayMask_));
    IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags_InputMask_));

    const bool alpha = (flags & ImGuiColorEditFlags_NoAlpha) == 0;
    const bool hdr = (flags & ImGuiColorEditFlags_HDR) != 0;
    const int components = alpha ? 4 : 3;

    float f[4] = { col[0], col[1], col[2], alpha ? col[3] : 1.0f };
    if ((flags & ImGuiColorEditFlags_InputHSV) && (flags & ImGuiColorEditFlags_DisplayRGB))
        ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
    else if ((flags & ImGuiColorEditFlags_InputRGB) && (flags & ImGuiColorEditFlags_DisplayHSV))
    {
        ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);
        ColorEditRestoreHS(col, &f[0], &f[1], &f[2]);
    }
    int i[4] = { IM_F32_TO_INT8_UNBOUND(f[0]), IM_F32_TO_INT8_UNBOUND(f[1]), IM_F32_TO_INT8_UNBOUND(f[2]), IM_F32_TO_INT8_UNBOUND(f[3]) };

    bool value_changed = false;
    bool value_changed_as_float = false;

    const ImVec2 pos = window->DC.CursorPos;
    const float inputs_offset_x = (style.ColorButtonPosition == ImGuiDir_Left) ? w_button : 0.0f;
    window->DC.CursorPos.x = pos.x + inputs_offset_x;

    if ((flags & (ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_DisplayHSV)) != 0 && (flags & ImGuiColorEditFlags_NoInputs) == 0)
    {
        const float w_item_one = ImMax(1.0f, IM_FLOOR((w_inputs - (style.ItemInnerSpacing.x) * (components - 1)) / (float)components));
        const float w_item_last = ImMax(1.0f, IM_FLOOR(w_inputs - (w_item_one + style.ItemInnerSpacing.x) * (components - 1)));

        const bool hide_prefix = (w_item_one <= CalcTextSize((flags & ImGuiColorEditFlags_Float) ? "M:0.000" : "M:000").x);
        static const char* ids[4] = { "##X", "##Y", "##Z", "##W" };
        static const char* fmt_table_int[3][4] =
        {
            {   "%3d",   "%3d",   "%3d",   "%3d" },
            { "R:%3d", "G:%3d", "B:%3d", "A:%3d" },
            { "H:%3d", "S:%3d", "V:%3d", "A:%3d" }
        };
        static const char* fmt_table_float[3][4] =
        {
            {   "%0.3f",   "%0.3f",   "%0.3f",   "%0.3f" },
            { "R:%0.3f", "G:%0.3f", "B:%0.3f", "A:%0.3f" },
            { "H:%0.3f", "S:%0.3f", "V:%0.3f", "A:%0.3f" }
        };
        const int fmt_idx = hide_prefix ? 0 : (flags & ImGuiColorEditFlags_DisplayHSV) ? 2 : 1;

        for (int n = 0; n < components; n++)
        {
            if (n > 0)
                SameLine(0, style.ItemInnerSpacing.x);
            SetNextItemWidth((n + 1 < components) ? w_item_one : w_item_last);

            if (flags & ImGuiColorEditFlags_Float)
            {
                value_changed |= DragFloat(ids[n], &f[n], 1.0f / 255.0f, 0.0f, hdr ? 0.0f : 1.0f, fmt_table_float[fmt_idx][n]);
                value_changed_as_float |= value_changed;
            }
            else
                value_changed |= DragInt(ids[n], &i[n], 1.0f, 0, hdr ? 0 : 255, fmt_table_int[fmt_idx][n]);

            if (!(flags & ImGuiColorEditFlags_NoOptions))
                OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);
        }
    }
    else if ((flags & ImGuiColorEditFlags_DisplayHex) != 0 && (flags & ImGuiColorEditFlags_NoInputs) == 0)
    {
        char buf[64];
        if (alpha)
            ImFormatString(buf, IM_ARRAYSIZE(buf), "#%02X%02X%02X%02X", ImClamp(i[0], 0, 255), ImClamp(i[1], 0, 255), ImClamp(i[2], 0, 255), ImClamp(i[3], 0, 255));
        else
            ImFormatString(buf, IM_ARRAYSIZE(buf), "#%02X%02X%02X", ImClamp(i[0], 0, 255), ImClamp(i[1], 0, 255), ImClamp(i[2], 0, 255));
        SetNextItemWidth(w_inputs);
        if (InputText("##Text", buf, IM_ARRAYSIZE(buf), ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase))
        {
            value_changed = true;
            char* p = buf;
            while (*p == '#' || ImCharIsBlankA(*p))
                p++;
            i[0] = i[1] = i[2] = 0;
            i[3] = 0xFF;
            int r;
            if (alpha)
                r = sscanf(p, "%02X%02X%02X%02X", (unsigned int*)&i[0], (unsigned int*)&i[1], (unsigned int*)&i[2], (unsigned int*)&i[3]);
            else
                r = sscanf(p, "%02X%02X%02X", (unsigned int*)&i[0], (unsigned int*)&i[1], (unsigned int*)&i[2]);
            IM_UNUSED(r);
        }
        if (!(flags & ImGuiColorEditFlags_NoOptions))
            OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);
    }

    ImGuiWindow* picker_active_window = NULL;
    if (!(flags & ImGuiColorEditFlags_NoSmallPreview))
    {
        window->DC.CursorPos = ImVec2(pos.x - square_sz + GetContentRegionMax().x - style.WindowPadding.x, pos.y);

        const ImVec4 col_v4(col[0], col[1], col[2], alpha ? col[3] : 1.0f);
        if (ColorButton("##ColorButton", col_v4, flags))
        {
            if (!(flags & ImGuiColorEditFlags_NoPicker))
            {
                g.ColorPickerRef = col_v4;
                OpenPopup("picker1");
                SetNextWindowPos(g.LastItemData.Rect.GetBL() + ImVec2(0.0f, style.ItemSpacing.y));
            }
        }
        if (!(flags & ImGuiColorEditFlags_NoOptions))
            OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);

        PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(13, 13));
        PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(8, 12));
        if (BeginPopup("picker1", ImGuiWindowFlags_NoResize))
        {
            if (g.CurrentWindow->BeginCount == 1)
            {
                picker_active_window = g.CurrentWindow;

                ImGuiColorEditFlags picker_flags_to_forward = ImGuiColorEditFlags_DataTypeMask_ | ImGuiColorEditFlags_PickerMask_ | ImGuiColorEditFlags_InputMask_ | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaBar;
                ImGuiColorEditFlags picker_flags = (flags_untouched & picker_flags_to_forward) | ImGuiColorEditFlags_DisplayMask_ | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview;
                SetNextItemWidth(square_sz * 10.0f);
                value_changed |= color_picker4("##picker", col, picker_flags, &g.ColorPickerRef.x);
            }
            EndPopup();
        }
        PopStyleVar(2);
    }

    float anim = float_lerp(GetID(label), picker_active_window);
    ImColor col_anim = color_lerp(GetColorU32(ImGuiCol_TextDisabled), GetColorU32(ImGuiCol_Text), anim);

    if (label != label_display_end && !(flags & ImGuiColorEditFlags_NoLabel))
        window->DrawList->AddText(pos, col_anim, label);

    if (value_changed && picker_active_window == NULL)
    {
        if (!value_changed_as_float)
            for (int n = 0; n < 4; n++)
                f[n] = i[n] / 255.0f;
        if ((flags & ImGuiColorEditFlags_DisplayHSV) && (flags & ImGuiColorEditFlags_InputRGB))
        {
            g.ColorEditSavedHue = f[0];
            g.ColorEditSavedSat = f[1];
            ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
            g.ColorEditSavedID = g.ColorEditCurrentID;
            g.ColorEditSavedColor = ColorConvertFloat4ToU32(ImVec4(f[0], f[1], f[2], 0));
        }
        if ((flags & ImGuiColorEditFlags_DisplayRGB) && (flags & ImGuiColorEditFlags_InputHSV))
            ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);

        col[0] = f[0];
        col[1] = f[1];
        col[2] = f[2];
        if (alpha)
            col[3] = f[3];
    }

    if (set_current_color_edit_id)
        g.ColorEditCurrentID = 0;

    PopID();
    EndGroup();

    if ((g.LastItemData.StatusFlags & ImGuiItemStatusFlags_HoveredRect) && !(flags & ImGuiColorEditFlags_NoDragDrop) && BeginDragDropTarget())
    {
        bool accepted_drag_drop = false;

        if (const ImGuiPayload* payload = AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F))
        {
            memcpy((float*)col, payload->Data, sizeof(float) * 3);
            value_changed = accepted_drag_drop = true;
        }

        if (const ImGuiPayload* payload = AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F))
        {
            memcpy((float*)col, payload->Data, sizeof(float) * components);
            value_changed = accepted_drag_drop = true;
        }

        if (accepted_drag_drop && (flags & ImGuiColorEditFlags_InputHSV))
            ColorConvertRGBtoHSV(col[0], col[1], col[2], col[0], col[1], col[2]);

        EndDragDropTarget();
    }

    if (picker_active_window && g.ActiveId != 0 && g.ActiveIdWindow == picker_active_window)
        g.LastItemData.ID = g.ActiveId;

    if (value_changed && g.LastItemData.ID != 0)
        MarkItemEdited(g.LastItemData.ID);

    return value_changed;
}

void widgets::color(const std::string name, Color_t& colValue)
{
    float color[4];
    colValue.BaseAlpha(color);

    if (widgets::color_edit4(name.c_str(), color, ALPHA))
        colValue = Color_t(color[COLOR_R], color[COLOR_G], color[COLOR_B], color[COLOR_A]);
}

bool widgets::slider_scalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;

    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 pos = window->DC.CursorPos;

    const float ww = GetContentRegionMax().x - style.WindowPadding.x;
    const float w = ww / 2.0f + GetFrameHeight();

    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    const ImRect bb(pos + ImVec2(ww - w, 0), pos + ImVec2(ww, label_size.y + style.FramePadding.y * 2.0f));
    const ImRect frame_bb(bb.Min + ImVec2(0, 3), bb.Max - ImVec2(0, 3));

    const bool temp_input_allowed = (flags & ImGuiSliderFlags_NoInput) == 0;
    ItemSize(bb, style.FramePadding.y);
    if (!ItemAdd(bb, id, &bb, temp_input_allowed ? ImGuiItemFlags_Inputable : 0))
        return false;

    if (format == NULL)
        format = DataTypeGetInfo(data_type)->PrintFmt;

    const bool hovered = ItemHoverable(frame_bb, id, g.LastItemData.InFlags);
    bool temp_input_is_active = temp_input_allowed && TempInputIsActive(id);
    if (!temp_input_is_active)
    {
        const bool input_requested_by_tabbing = temp_input_allowed && (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_FocusedByTabbing) != 0;
        const bool clicked = hovered && IsMouseClicked(0, id);
        const bool make_active = (input_requested_by_tabbing || clicked || g.NavActivateId == id);

        if (make_active && clicked)
            SetKeyOwner(ImGuiKey_MouseLeft, id);

        if (make_active && temp_input_allowed)
            if (input_requested_by_tabbing || (clicked && g.IO.KeyCtrl) || (g.NavActivateId == id && (g.NavActivateFlags & ImGuiActivateFlags_PreferInput)))
                temp_input_is_active = true;

        if (make_active && !temp_input_is_active)
        {
            SetActiveID(id, window);
            SetFocusID(id, window);
            FocusWindow(window);
            g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
        }
    }

    if (temp_input_is_active)
    {
        const bool is_clamp_input = (flags & ImGuiSliderFlags_AlwaysClamp) != 0;
        return TempInputScalar(frame_bb, id, label, data_type, p_data, format, is_clamp_input ? p_min : NULL, is_clamp_input ? p_max : NULL);
    }

    ImRect grab_bb;
    const bool value_changed = SliderBehavior(frame_bb, id, data_type, p_data, p_min, p_max, format, flags, &grab_bb);
    if (value_changed)
        MarkItemEdited(id);

    float filled_width = grab_bb.Max.x - frame_bb.Min.x;
    window->DrawList->AddRectFilled(frame_bb.Min + ImVec2(1.0f, 1.0f), ImVec2(frame_bb.Min.x + filled_width, frame_bb.Max.y - 1.0f), GetColorU32(ImGuiCol_FrameBg), g.Style.FrameRounding);
    window->DrawList->AddRect(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_Border), g.Style.FrameRounding);

    char value_buf[64];
    const char* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);
    if (g.LogEnabled)
        LogSetNextTextDecoration("{", "}");
    RenderTextClipped(bb.Min, bb.Max, value_buf, value_buf_end, NULL, ImVec2(0.5f, 0.5f));

    float anim = float_lerp(id, g.ActiveId == id);
    ImColor col = color_lerp(GetColorU32(ImGuiCol_TextDisabled), GetColorU32(ImGuiCol_Text), anim);

    if (label_size.x > 0.0f)
        window->DrawList->AddText(ImVec2(pos.x, pos.y + style.FramePadding.y), col, label);

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | (temp_input_allowed ? ImGuiItemStatusFlags_Inputable : 0));
    return value_changed;
}

bool widgets::slider_int(const char* label, int* v, int v_min, int v_max, const char* format, ImGuiSliderFlags flags)
{
    return slider_scalar(label, ImGuiDataType_S32, v, &v_min, &v_max, format, flags);
}

bool widgets::slider_float(const char* label, float* v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
{
    return slider_scalar(label, ImGuiDataType_Float, v, &v_min, &v_max, format, flags);
}

bool widgets::multicombo(const char* label, bool* values, const char* const items[], int items_count, int height_in_items)
{
    ImGuiContext& g = *GImGui;

    char preview_text[256] = { 0 };
    bool first_selected = true;
    for (int i = 0; i < items_count; i++)
    {
        if (values[i])
        {
            if (!first_selected)
                strcat(preview_text, ", ");
            strcat(preview_text, items[i]);
            first_selected = false;
        }
    }

    if (first_selected)
        strcpy(preview_text, "None");

    if (height_in_items != -1 && !(g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSizeConstraint))
        SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, CalcMaxPopupHeightFromItemCount(height_in_items)));

    if (!widgets::begin_combo(label, preview_text, ImGuiComboFlags_None))
        return false;

    bool value_changed = false;
    for (int i = 0; i < items_count; i++)
    {
        PushID(i);
        if (widgets::selectable(items[i], values[i], ImGuiSelectableFlags_DontClosePopups))
        {
            values[i] = !values[i];
            value_changed = true;
        }
        PopID();
    }

    EndCombo();

    if (value_changed)
        MarkItemEdited(g.LastItemData.ID);

    return value_changed;
}

const char* keys[] =
{
    "None",
    "Mouse 1",
    "Mouse 2",
    "CN",
    "Mouse 3",
    "Mouse 4",
    "Mouse 5",
    "-",
    "Back",
    "Tab",
    "-",
    "-",
    "CLR",
    "Enter",
    "-",
    "-",
    "Shift",
    "CTL",
    "Menu",
    "Pause",
    "Caps Lock",
    "KAN",
    "-",
    "JUN",
    "FIN",
    "KAN",
    "-",
    "Escape",
    "CON",
    "NCO",
    "ACC",
    "MAD",
    "Space",
    "PGU",
    "PGD",
    "End",
    "Home",
    "Left",
    "Up",
    "Right",
    "Down",
    "SEL",
    "PRI",
    "EXE",
    "PRI",
    "INS",
    "Delete",
    "HEL",
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "-",
    "-",
    "-",
    "-",
    "-",
    "-",
    "-",
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "WIN",
    "WIN",
    "APP",
    "-",
    "SLE",
    "Numpad 0",
    "Numpad 1",
    "Numpad 2",
    "Numpad 3",
    "Numpad 4",
    "Numpad 5",
    "Numpad 6",
    "Numpad 7",
    "Numpad 8",
    "Numpad 9",
    "MUL",
    "ADD",
    "SEP",
    "MIN",
    "Delete",
    "DIV",
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "F7",
    "F8",
    "F9",
    "F10",
    "F11",
    "F12",
    "F13",
    "F14",
    "F15",
    "F16",
    "F17",
    "F18",
    "F19",
    "F20",
    "F21",
    "F22",
    "F23",
    "F24",
    "-",
    "-",
    "-",
    "-",
    "-",
    "-",
    "-",
    "-",
    "NUM",
    "SCR",
    "EQU",
    "MAS",
    "TOY",
    "OYA",
    "OYA",
    "-",
    "-",
    "-",
    "-",
    "-",
    "-",
    "-",
    "-",
    "-",
    "Shift",
    "Shift",
    "Ctrl",
    "Ctrl",
    "Alt",
    "Alt"
};

#include <string>
#include <chrono>

struct key_state
{
    ImVec4 background, text;

    bool active = false;
    bool hovered = false;

    float alpha = 0.f;
};

bool widgets::keybind(const char* label, int* key, int* mode)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;
    const ImGuiStyle& style = g.Style;

    const ImGuiID id = window->GetID(label);
    const float width = (GetContentRegionMax().x - style.WindowPadding.x);

    const ImRect rect(window->DC.CursorPos, window->DC.CursorPos + ImVec2(width, 22.f));

    ItemSize(ImRect(rect.Min, rect.Max - ImVec2(0, 2)));
    if (!ImGui::ItemAdd(rect, id))
        return false;

    char buf_display[64] = "None";

    bool value_changed = false;
    int k = *key;

    std::string active_key = "";
    active_key += keys[*key];

    if (*key != 0 && g.ActiveId != id)
        strcpy_s(buf_display, active_key.c_str());
    else if (g.ActiveId == id)
        strcpy_s(buf_display, "...");

    const ImVec2 label_size = CalcTextSize(buf_display, NULL, true);

    ImRect clickable(ImVec2(rect.Max.x - 10 - label_size.x, rect.Min.y), rect.Max);
    bool hovered = ItemHoverable(clickable, id, 0);

    static std::map<ImGuiID, key_state> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, key_state() });
        it_anim = anim.find(id);
    }

    ImVec4 _keyi_bg_hov = ImColor(87, 83, 78, 75);
    ImVec4 _keyii_bg = ImColor(87, 83, 78, 45);

    ImVec4 text_hov = ImColor(255, 255, 255, 150);
    ImVec4 text_active = ImColor(255, 255, 255, 255);
    ImVec4 text = ImColor(255, 255, 255, 100);

    it_anim->second.background = ImLerp(it_anim->second.background, g.ActiveId == id || hovered ? _keyi_bg_hov : _keyii_bg, ImGui::GetIO().DeltaTime * 6.f);
    it_anim->second.text = ImLerp(it_anim->second.text, g.ActiveId == id ? text_active : hovered ? text_hov : text, ImGui::GetIO().DeltaTime * 6.f);

    window->DrawList->AddRectFilled(clickable.Min, clickable.Max, GetColorU32(it_anim->second.background), 4);
    window->DrawList->AddText(rect.Min - ImVec2(0, 0), GetColorU32(it_anim->second.text), label);

    PushStyleColor(ImGuiCol_Text, text_active);
    RenderTextClipped(clickable.Min - ImVec2(0, 1), clickable.Max - ImVec2(0, 1), buf_display, NULL, &label_size, ImVec2(0.5f, 0.5f));
    PopStyleColor();

    if (hovered && io.MouseClicked[0])
    {
        if (g.ActiveId != id) {
            memset(io.MouseDown, 0, sizeof(io.MouseDown));
            memset(io.KeysDown, 0, sizeof(io.KeysDown));
            *key = 0;
        }

        ImGui::SetActiveID(id, window);
        ImGui::FocusWindow(window);
    }
    else if (io.MouseClicked[0]) {
        if (g.ActiveId == id)
            ImGui::ClearActiveID();
    }

    if (g.ActiveId == id) {
        for (auto i = 0; i < 5; i++) {
            if (io.MouseDown[i]) {
                switch (i) {
                case 0:
                    k = 0x01;
                    break;
                case 1:
                    k = 0x02;
                    break;
                case 2:
                    k = 0x04;
                    break;
                case 3:
                    k = 0x05;
                    break;
                case 4:
                    k = 0x06;
                    break;
                }

                value_changed = true;
                ImGui::ClearActiveID();
            }
        }
        if (!value_changed) {
            for (auto i = 0x08; i <= 0xA5; i++) {
                if (io.KeysDown[i]) {
                    k = i;
                    value_changed = true;
                    ImGui::ClearActiveID();
                }
            }
        }

        if (IsKeyPressedMap(ImGuiKey_Escape)) {
            *key = 0;
            ImGui::ClearActiveID();
        }
        else
            *key = k;
    }

    if (hovered && g.IO.MouseClicked[1] || it_anim->second.active && (g.IO.MouseClicked[0] || g.IO.MouseClicked[1]) && !it_anim->second.hovered)
        it_anim->second.active = !it_anim->second.active;

    it_anim->second.alpha = ImClamp(it_anim->second.alpha + (8.f * g.IO.DeltaTime * (it_anim->second.active ? 1.f : -1.f)), 0.f, 1.f);

    ImVec4 i_bg = ImColor(87, 83, 78, 45);

    PushStyleColor(ImGuiCol_WindowBg, i_bg);

    if (BeginPopup(label, ImGuiWindowFlags_NoResize) && it_anim->second.alpha >= 0.01f)
    {
        BeginGroup();
        {
            it_anim->second.hovered = IsWindowHovered();

            if (widgets::selectable("Hold", *mode == 0))
            {
                *mode = 0;
                it_anim->second.active = false;
            }
            if (widgets::selectable("Toggle", *mode == 1))
            {
                *mode = 1;
                it_anim->second.active = false;
            }
            if (widgets::selectable("Always", *mode == 2))
            {
                *mode = 2;
                it_anim->second.active = false;
            }
        }
        EndGroup();

        EndPopup();
    }

    if (hovered && io.MouseClicked[1])
        OpenPopup(label);

    PopStyleColor();

    return value_changed;
}

void widgets::add_log(const std::string& msg) {
    cur_logs.push_back(NotifyStruct(msg, ImGui::GetTime()));
}

void widgets::render_log()
{
    const float l_time = 3.0f;
    float c_time = ImGui::GetTime();

    // remove old log
    cur_logs.erase(std::remove_if(cur_logs.begin(), cur_logs.end(),
        [c_time, l_time](const NotifyStruct& log)
        {
            return (c_time - log.time) > l_time;
        }
    ), cur_logs.end());

    float w_h = 10.f + (cur_logs.size() * 20.f);

    // set size/pos
    ImGui::SetNextWindowSize(ImVec2(350, w_h), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(5, 5));

    if (ImGui::Begin(("##MenuLogs"), NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove))
    {
        for (const auto& log : cur_logs)
        {
            float left = l_time - (c_time - log.time);
            float alpha = ImClamp(left, 0.1f, 1.0f);

            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
            ImGui::TextUnformatted(log.msg.c_str());
            ImGui::PopStyleVar();
        }
    }
    ImGui::End();
}