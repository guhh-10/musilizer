#include <imgui.h>
#include <imgui_internal.h>
#include <IconsLucide.h>

#include "ui/imgui_widgets.hpp"

// ── Font resolver ───────────────────────────────────────────────────────────

ImFont* resolveFont(ButtonFont font)
{
    switch (font) {
        case ButtonFont::Bold:       return FontManager::bold();
        case ButtonFont::Icons:      return FontManager::icons();
        case ButtonFont::LargeIcons: return FontManager::largeIcons();
        case ButtonFont::Regular:
        default:                     return FontManager::regular();
    }
}

// ── SmoothRadioButton ───────────────────────────────────────────────────────

bool SmoothRadioButton(const char* label, const ImVec2& size_arg, ButtonFont font, bool active)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    ImGui::PushFont(resolveFont(font));

    ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    // This 'size' is now treated as the absolute MAXIMUM target size
    ImVec2 size = ImVec2(
        (size_arg.x == 0.0f) ? (label_size.x + style.FramePadding.x * 2.0f) : size_arg.x,
        (size_arg.y == 0.0f) ? (label_size.y + style.FramePadding.y * 2.0f) : size_arg.y
    );

    // The invisible hit-box remains at max size so the mouse interaction zone doesn't constantly shift
    const ImRect bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + size.x, window->DC.CursorPos.y + size.y));
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id)) {
        ImGui::PopFont();
        return false;
    }

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    // Trackers: t_hover strictly controls Sizing, while the Color tracker is shared
    float& t_hover = SmoothAnimState::GetRef(id + 1);
    float& t_color = SmoothAnimState::GetRef(id + 2);
    float animation_speed = 12.0f;

    // Size Scale Tracker (only pops up slightly on physical mouse hover)
    if (hovered) {
        t_hover += g.IO.DeltaTime * animation_speed;
        if (t_hover > 1.0f) t_hover = 1.0f;
    } else {
        t_hover -= g.IO.DeltaTime * animation_speed;
        if (t_hover < 0.0f) t_hover = 0.0f;
    }

    // COMBINED Color Tracker: Triggers color change if hovered OR if active
    if (hovered || active) {
        t_color += g.IO.DeltaTime * animation_speed;
        if (t_color > 1.0f) t_color = 1.0f;
    } else {
        t_color -= g.IO.DeltaTime * animation_speed;
        if (t_color < 0.0f) t_color = 0.0f;
    }

    ImVec2 center = ImVec2((bb.Min.x + bb.Max.x) * 0.5f, (bb.Min.y + bb.Max.y) * 0.5f);
    
    // Idle (t_hover = 0): scale is 0.90 (90% size)
    // Hovered (t_hover = 1): scale is 1.00 (100% max size)
    float current_scale = 0.90f + (0.10f * t_hover);
    
    ImVec2 scaled_size = ImVec2(size.x * current_scale, size.y * current_scale);

    ImRect visual_bb(
        ImVec2(center.x - scaled_size.x * 0.5f, center.y - scaled_size.y * 0.5f),
        ImVec2(center.x + scaled_size.x * 0.5f, center.y + scaled_size.y * 0.5f)
    );

    ImVec4 col_normal = style.Colors[ImGuiCol_Button];
    ImVec4 col_active = style.Colors[ImGuiCol_SliderGrab];

    ImU32 bg_col = ImGui::GetColorU32(
        held ? style.Colors[ImGuiCol_ButtonActive] :
        ImVec4(col_normal.x + (col_active.x - col_normal.x) * t_color,
               col_normal.y + (col_active.y - col_normal.y) * t_color,
               col_normal.z + (col_active.z - col_normal.z) * t_color,
               col_normal.w + (col_active.w - col_normal.w) * t_color)
    );

    window->DrawList->AddRectFilled(visual_bb.Min, visual_bb.Max, bg_col, style.FrameRounding);

    // --- FIXED JITTER TEXT RENDERING ---
    // We pass the unmoving structural 'bb' bounds for text centering alignment calculation targets.
    // We pass the dynamic 'visual_bb' as the final parameter so text clips cleanly to the shrinking edge boundary.
    ImGui::RenderTextClipped(
        bb.Min, bb.Max,
        label, NULL, &label_size, style.ButtonTextAlign, &visual_bb
    );

    ImGui::PopFont();

    return pressed;
}

// ── SmoothSliderBare ────────────────────────────────────────────────────────

bool SmoothSliderBare(const char* str_id, float* v, float v_min, float v_max, float slider_bar_width)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(str_id);

    float base_track_height = 4.0f;
    float max_track_height  = 8.0f;
    float base_grab_radius  = 0.0f; 
    float max_grab_radius   = 6.0f; 

    ImVec2 pos = window->DC.CursorPos;
    ImRect slider_bb(pos, ImVec2(pos.x + slider_bar_width, pos.y + 20.0f)); 

    ImGui::ItemSize(slider_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(slider_bb, id)) return false;

    bool hovered = ImGui::IsMouseHoveringRect(slider_bb.Min, slider_bb.Max);
    bool held = false;
    ImGui::ButtonBehavior(slider_bb, id, &hovered, &held);

    // Follows your precise SmoothAnimState map lookup convention
    float& t = SmoothAnimState::GetRef(id);
    float animation_speed = 14.0f; 
    if (hovered || held) {
        t += g.IO.DeltaTime * animation_speed;
        if (t > 1.0f) t = 1.0f;
    } else {
        t -= g.IO.DeltaTime * animation_speed;
        if (t < 0.0f) t = 0.0f;
    }

    if (held) {
        float clicked_t = (g.IO.MousePos.x - slider_bb.Min.x) / slider_bb.GetWidth();
        *v = v_min + ImClamp(clicked_t, 0.0f, 1.0f) * (v_max - v_min);
    }

    float fraction = (*v - v_min) / (v_max - v_min);
    float current_track_height = base_track_height + (max_track_height - base_track_height) * t;
    float centerY = (slider_bb.Min.y + slider_bb.Max.y) * 0.5f;

    ImRect visual_track_bb(
        ImVec2(slider_bb.Min.x, centerY - (current_track_height * 0.5f)),
        ImVec2(slider_bb.Max.x, centerY + (current_track_height * 0.5f))
    );

    // Follows your custom theme mapping variables
    ImU32 bg_color   = ImGui::GetColorU32(style.Colors[ImGuiCol_FrameBg]);
    ImU32 fill_color = ImGui::GetColorU32(style.Colors[ImGuiCol_SliderGrab]);
    ImU32 grab_color = ImGui::GetColorU32(style.Colors[ImGuiCol_ButtonHovered]);

    window->DrawList->AddRectFilled(visual_track_bb.Min, visual_track_bb.Max, bg_color, 99.0f);

    float fill_width = visual_track_bb.GetWidth() * fraction;
    if (fill_width > 0.0f) {
        ImRect filled_segment(visual_track_bb.Min, ImVec2(visual_track_bb.Min.x + fill_width, visual_track_bb.Max.y));
        window->DrawList->AddRectFilled(filled_segment.Min, filled_segment.Max, fill_color, 99.0f);
    }

    float current_grab_radius = base_grab_radius + (max_grab_radius - base_grab_radius) * t;
    if (current_grab_radius > 0.0f) {
        ImVec2 grab_center = ImVec2(visual_track_bb.Min.x + fill_width, centerY);
        window->DrawList->AddCircleFilled(grab_center, current_grab_radius, grab_color, 16);
    }

    return held;
}

bool SmoothScaleButton(const char* label, const ImVec2& size_arg, ButtonFont font)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    ImGui::PushFont(resolveFont(font));
    ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    ImVec2 size = ImVec2(
        (size_arg.x == 0.0f) ? (label_size.x + style.FramePadding.x * 2.0f) : size_arg.x,
        (size_arg.y == 0.0f) ? (label_size.y + style.FramePadding.y * 2.0f) : size_arg.y
    );

    const ImRect bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + size.x, window->DC.CursorPos.y + size.y));
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id)) {
        ImGui::PopFont();
        return false;
    }

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    // Smooth animation tracking using SmoothAnimState
    // We use id + 1 for size scaling and id + 2 for color mixing to isolate states cleanly
    float& t_scale = SmoothAnimState::GetRef(id + 1);
    float& t_color = SmoothAnimState::GetRef(id + 2);
    float animation_speed = 12.0f;

    // Sizing scale tracker
    if (hovered) { t_scale += g.IO.DeltaTime * animation_speed; if (t_scale > 1.0f) t_scale = 1.0f; }
    else         { t_scale -= g.IO.DeltaTime * animation_speed; if (t_scale < 0.0f) t_scale = 0.0f; }

    // Color transition tracker
    if (hovered) { t_color += g.IO.DeltaTime * animation_speed; if (t_color > 1.0f) t_color = 1.0f; }
    else         { t_color -= g.IO.DeltaTime * animation_speed; if (t_color < 0.0f) t_color = 0.0f; }

    // Visual Scaling Math
    ImVec2 center = ImVec2((bb.Min.x + bb.Max.x) * 0.5f, (bb.Min.y + bb.Max.y) * 0.5f);
    float current_scale = 0.90f + (0.10f * t_scale);
    ImVec2 scaled_size = ImVec2(size.x * current_scale, size.y * current_scale);
    ImRect visual_bb(
        ImVec2(center.x - scaled_size.x * 0.5f, center.y - scaled_size.y * 0.5f),
        ImVec2(center.x + scaled_size.x * 0.5f, center.y + scaled_size.y * 0.5f)
    );

    // Color Interpolation Logic (Matches SmoothRadioButton convention)
    ImVec4 col_normal = style.Colors[ImGuiCol_Button];
    ImVec4 col_active = style.Colors[ImGuiCol_SliderGrab]; // Or use ImGuiCol_ButtonHovered depending on preference

    ImU32 bg_col = ImGui::GetColorU32(
        held ? style.Colors[ImGuiCol_ButtonActive] :
        ImVec4(col_normal.x + (col_active.x - col_normal.x) * t_color,
               col_normal.y + (col_active.y - col_normal.y) * t_color,
               col_normal.z + (col_active.z - col_normal.z) * t_color,
               col_normal.w + (col_active.w - col_normal.w) * t_color)
    );

    // Render background
    ImGui::PopFont();
    window->DrawList->AddRectFilled(visual_bb.Min, visual_bb.Max, bg_col, style.FrameRounding);
    
    // Render text clipped inside the dynamic visual bounds
    ImGui::PushFont(resolveFont(font));
    ImGui::RenderTextClipped(bb.Min, bb.Max, label, NULL, &label_size, style.ButtonTextAlign, &visual_bb);
    ImGui::PopFont();

    return pressed;
}

bool SmoothActiveInputText(const char* label, char* buf, size_t buf_size, const ImVec2& size_arg)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext& g = *GImGui;
    const ImGuiID id = window->GetID(label);

    float& t = SmoothAnimState::GetRef(id);

    // Color mixing (Border baseline vs. Slider/Accent color glow)
    ImVec4 border_normal = g.Style.Colors[ImGuiCol_Border];
    ImVec4 border_active = g.Style.Colors[ImGuiCol_SliderGrab]; 

    ImVec4 mixed_border_col = ImVec4(
        border_normal.x + (border_active.x - border_normal.x) * t,
        border_normal.y + (border_active.y - border_normal.y) * t,
        border_normal.z + (border_active.z - border_normal.z) * t,
        border_normal.w + (border_active.w - border_normal.w) * t
    );

    ImGui::PushStyleColor(ImGuiCol_Border, mixed_border_col);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f); 

    // Set layout sizing configuration if supplied explicitly
    if (size_arg.x > 0.0f) ImGui::PushItemWidth(size_arg.x);

    bool changed = ImGui::InputText(label, buf, buf_size);
    
    if (size_arg.x > 0.0f) ImGui::PopItemWidth();
    ImGui::PopStyleVar();

    // Smooth active/focused state tracking
    bool is_active = ImGui::IsItemActive();
    float animation_speed = 10.0f;
    if (is_active) {
        t += g.IO.DeltaTime * animation_speed;
        if (t > 1.0f) t = 1.0f;
    } else {
        t -= g.IO.DeltaTime * animation_speed;
        if (t < 0.0f) t = 0.0f;
    }

    ImGui::PopStyleColor(1);
    return changed;
}

// ── SmoothHoverTable ────────────────────────────────────────────────────────

bool SmoothHoverTable(const char* str_id, const std::vector<TableRowItem>& items, int* out_selected_index)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;
 
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
 
    ImGuiTableFlags table_flags = ImGuiTableFlags_NoBordersInBody
                                | ImGuiTableFlags_NoHostExtendX;
 
    bool any_clicked = false;
 
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(8.0f, 7.0f));
 
    if (ImGui::BeginTable(str_id, 3, table_flags, ImVec2(0, 0)))
    {
        ImGui::TableSetupColumn("Number",   ImGuiTableColumnFlags_WidthFixed,   30.0f);
        ImGui::TableSetupColumn("Title",    ImGuiTableColumnFlags_WidthStretch,  1.0f);
        ImGui::TableSetupColumn("Duration", ImGuiTableColumnFlags_WidthFixed,   45.0f);
 
        for (int i = 0; i < (int)items.size(); ++i)
        {
            ImGui::TableNextRow();
 
            ImGuiID row_id = ImGui::GetID((std::string(str_id) + "_row_" + std::to_string(i)).c_str());
            float& t = SmoothAnimState::GetRef(row_id);
            float animation_speed = 12.0f;
 
            // Push transparent colors so the Selectable itself draws no highlight
            ImGui::PushStyleColor(ImGuiCol_Header,        IM_COL32(0,0,0,0));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(0,0,0,0));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive,  IM_COL32(0,0,0,0));
 
            ImGui::TableSetColumnIndex(0);
 
            std::string selectable_id = "##row_sel_" + std::to_string(i);
            bool is_selected = (out_selected_index && *out_selected_index == i);
 
            if (ImGui::Selectable(selectable_id.c_str(), is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
                if (out_selected_index) *out_selected_index = i;
                any_clicked = true;
            }
 
            bool row_hovered = ImGui::IsItemHovered();
 
            ImGui::PopStyleColor(3);
 
            // --- Smooth hover interpolation ---
            if (row_hovered) {
                t += g.IO.DeltaTime * animation_speed;
                if (t > 1.0f) t = 1.0f;
            } else {
                t -= g.IO.DeltaTime * animation_speed;
                if (t < 0.0f) t = 0.0f;
            }
 
            // --- Column 0: Center-aligned Number text ---
            ImGui::TableSetColumnIndex(0);
            ImGui::SameLine();

            // Calculate text width to center it within the 30.0f fixed column width
            float text_width = ImGui::CalcTextSize(items[i].number.c_str()).x;
            float column_width = ImGui::GetContentRegionAvail().x; 
            if (text_width < column_width)
            {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (column_width - text_width) * 0.5f);
            }
            ImGui::TextUnformatted(items[i].number.c_str());
 
            // --- Column 1: Title ---
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(items[i].title.c_str());
 
            // --- Column 2: Duration ---
            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(items[i].duration.c_str());

            // --- Draw highlight AFTER all columns are rendered, but on the background channel ---
            if (t > 0.01f) {
                ImGuiTable* table = g.CurrentTable;
                const float rounding = 6.0f;
                const float inset_x  = 4.0f;
                const float inset_y  = 2.0f;
 
                ImVec2 min_p = ImVec2(table->WorkRect.Min.x + inset_x, table->RowPosY1 + inset_y);
                ImVec2 max_p = ImVec2(table->WorkRect.Max.x - inset_x, table->RowPosY2 - inset_y);
 
                ImVec4 col_bg = style.Colors[ImGuiCol_HeaderHovered];
                ImU32 row_bg_color = ImGui::GetColorU32(ImVec4(col_bg.x, col_bg.y, col_bg.z, col_bg.w * t));
 
                ImGui::TablePushBackgroundChannel();
                window->DrawList->AddRectFilled(min_p, max_p, row_bg_color, rounding);
                ImGui::TablePopBackgroundChannel();
            }
        }
 
        ImGui::EndTable();
    }
 
    ImGui::PopStyleVar();
 
    return any_clicked;
}

void RenderPlaylistGroupNode(const PlaylistGroup& group, const std::string& base_id, int group_idx, ImGuiID& global_active_id)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    const ImGuiStyle& style = g.Style;
    ImDrawList* draw_list = window->DrawList;

    float row_height = 24.0f;
    float header_height = 32.0f; // Increased header height (was 24.0f)
    float animation_speed = 12.0f;
    float content_width = ImGui::GetContentRegionAvail().x;

    // 1. RENDER TOP-LEVEL PLAYLIST FOLDER (Full Row Highlight)
    std::string folder_id_str = base_id + "_f_" + std::to_string(group_idx);
    ImGuiID folder_id = window->GetID(folder_id_str.c_str());

    float& f_hover = SmoothAnimState::GetRef(folder_id);
    float& f_active = SmoothAnimState::GetRef(folder_id + 555);
    bool& folder_open = *(bool*)&SmoothAnimState::GetRef(folder_id + 999);

    ImVec2 folder_pos = window->DC.CursorPos;
    ImRect folder_bb(folder_pos, ImVec2(folder_pos.x + content_width, folder_pos.y + header_height)); // Use header_height

    ImGui::ItemSize(folder_bb, style.FramePadding.y);
    if (ImGui::ItemAdd(folder_bb, folder_id)) 
    {
        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(folder_bb, folder_id, &hovered, &held);

        if (pressed) {
            global_active_id = folder_id; 
            folder_open = !folder_open;   
        }

        f_hover = ImClamp(f_hover + (hovered ? g.IO.DeltaTime : -g.IO.DeltaTime) * animation_speed, 0.0f, 1.0f);
        f_active = ImClamp(f_active + ((global_active_id == folder_id) ? g.IO.DeltaTime : -g.IO.DeltaTime) * animation_speed, 0.0f, 1.0f);

        ImVec4 col_hover = style.Colors[ImGuiCol_HeaderHovered];
        ImVec4 col_active = style.Colors[ImGuiCol_HeaderActive];
        if (col_active.w == 0.0f) col_active = style.Colors[ImGuiCol_SliderGrab];

        float bg_alpha = (col_hover.w * f_hover) * (1.0f - f_active) + (col_active.w * 0.4f) * f_active;
        if (bg_alpha > 0.01f) {
            ImVec4 mixed_bg = ImVec4(
                col_hover.x + (col_active.x - col_hover.x) * f_active,
                col_hover.y + (col_active.y - col_hover.y) * f_active,
                col_hover.z + (col_active.z - col_hover.z) * f_active,
                bg_alpha
            );
            draw_list->AddRectFilled(folder_bb.Min, folder_bb.Max, ImGui::GetColorU32(mixed_bg), 3.0f);
        }

        float center_y = folder_bb.Min.y + (header_height * 0.5f); // Use header_height
        float text_offset_y = center_y - (ImGui::GetTextLineHeight() * 0.5f);
        float x_cursor = folder_bb.Min.x + 6.0f;

        ImVec4 col_text_normal = style.Colors[ImGuiCol_Text];
        ImVec4 col_text_accent = style.Colors[ImGuiCol_SliderGrabActive];

        float arrow_size = 5.0f; // Slightly larger arrow for bigger header
        ImU32 arrow_color = ImGui::GetColorU32(ImVec4(
            col_text_normal.x + (col_text_accent.x - col_text_normal.x) * f_active,
            col_text_normal.y + (col_text_accent.y - col_text_normal.y) * f_active,
            col_text_normal.z + (col_text_accent.z - col_text_normal.z) * f_active,
            1.0f
        ));

        if (folder_open) {
            draw_list->AddTriangleFilled(ImVec2(x_cursor, center_y - arrow_size * 0.5f), ImVec2(x_cursor + arrow_size * 2.0f, center_y - arrow_size * 0.5f), ImVec2(x_cursor + arrow_size, center_y + arrow_size * 0.5f), arrow_color);
        } else {
            draw_list->AddTriangleFilled(ImVec2(x_cursor, center_y - arrow_size), ImVec2(x_cursor + arrow_size * 0.86f * 2.0f, center_y), ImVec2(x_cursor, center_y + arrow_size), arrow_color);
        }
        x_cursor += 14.0f;

        // --- NEW: Render Lucide ICON_LC_LIST_MUSIC Icon ---
        ImU32 icon_color = ImGui::GetColorU32(ImVec4(
            col_text_normal.x + (col_text_accent.x - col_text_normal.x) * f_active,
            col_text_normal.y + (col_text_accent.y - col_text_normal.y) * f_active,
            col_text_normal.z + (col_text_accent.z - col_text_normal.z) * f_active,
            1.0f
        ));

        // Use the s_icons font from your FontManager namespace/class
        ImGui::PushFont(FontManager::icons());
        float icon_text_offset_y = center_y - (ImGui::GetTextLineHeight() * 0.5f);
        draw_list->AddText(ImVec2(x_cursor, icon_text_offset_y), icon_color, ICON_LC_LIST_MUSIC);
        
        // Advance cursor past the icon (roughly 18 pixels for clean alignment layout)
        float icon_width = ImGui::CalcTextSize(ICON_LC_LIST_MUSIC).x;
        ImGui::PopFont();
        
        x_cursor += icon_width + 6.0f; 
        // --------------------------------------------------

        std::string count_str = std::to_string(group.tracks.size()) + (group.tracks.size() == 1 ? " track" : " tracks");
        ImVec2 count_text_size = ImGui::CalcTextSize(count_str.c_str());
        float count_x_pos = folder_bb.Max.x - count_text_size.x - 8.0f;
        
        ImU32 final_count_col = ImGui::GetColorU32(ImVec4(
            style.Colors[ImGuiCol_TextDisabled].x + (col_text_accent.x - style.Colors[ImGuiCol_TextDisabled].x) * f_active * 0.5f,
            style.Colors[ImGuiCol_TextDisabled].y + (col_text_accent.y - style.Colors[ImGuiCol_TextDisabled].y) * f_active * 0.5f,
            style.Colors[ImGuiCol_TextDisabled].z + (col_text_accent.z - style.Colors[ImGuiCol_TextDisabled].z) * f_active * 0.5f,
            style.Colors[ImGuiCol_TextDisabled].w
        ));
        draw_list->AddText(ImVec2(count_x_pos, text_offset_y), final_count_col, count_str.c_str());

        float max_title_width = count_x_pos - x_cursor - 8.0f; 
        if (max_title_width > 0.0f) 
        {
            ImU32 final_folder_text_col = ImGui::GetColorU32(ImVec4(
                col_text_normal.x + (col_text_accent.x - col_text_normal.x) * f_active,
                col_text_normal.y + (col_text_accent.y - col_text_normal.y) * f_active,
                col_text_normal.z + (col_text_accent.z - col_text_normal.z) * f_active,
                1.0f
            ));

            ImVec2 text_min = ImVec2(x_cursor, folder_bb.Min.y);
            ImVec2 text_max = ImVec2(x_cursor + max_title_width, folder_bb.Max.y);
            
            ImGui::PushStyleColor(ImGuiCol_Text, final_folder_text_col);
            ImGui::RenderTextClipped(text_min, text_max, group.name.c_str(), nullptr, nullptr, ImVec2(0.0f, 0.5f));
            ImGui::PopStyleColor();
        }

        if (ImGui::BeginPopupContextItem(folder_id_str.c_str())) {
            ImGui::TextColored(ImVec4(0.95f, 0.75f, 0.30f, 1.0f), "Playlist Options");
            ImGui::Separator();
            if (ImGui::MenuItem("Play Entire Playlist Selection")) {}
            ImGui::EndPopup();
        }
    }

    // 2. RENDER INNER TRACK ENTRIES
    if (folder_open) {
        for (int t = 0; t < (int)group.tracks.size(); t++) {
            const auto& track = group.tracks[t];
            std::string track_id_str = folder_id_str + "_t_" + std::to_string(t);
            ImGuiID track_id = window->GetID(track_id_str.c_str());

            float& t_hover = SmoothAnimState::GetRef(track_id);

            ImVec2 track_pos = window->DC.CursorPos;
            ImRect track_bb(track_pos, ImVec2(track_pos.x + content_width, track_pos.y + row_height));

            ImGui::ItemSize(track_bb, style.FramePadding.y);
            if (!ImGui::ItemAdd(track_bb, track_id)) continue;

            bool t_hovered, t_held;
            bool t_pressed = ImGui::ButtonBehavior(track_bb, track_id, &t_hovered, &t_held);

            if (t_pressed && ImGui::IsMouseDoubleClicked(0)) {
                printf("[Audio Engine Master] Stream Track: %s\n", track.name.c_str());
            }

            t_hover = ImClamp(t_hover + (t_hovered ? g.IO.DeltaTime : -g.IO.DeltaTime) * animation_speed, 0.0f, 1.0f);

            float t_content_start_x = track_bb.Min.x + 22.0f; 

            if (t_hover > 0.01f) {
                ImVec4 col_h = style.Colors[ImGuiCol_HeaderHovered];
                ImU32 track_bg_col = ImGui::GetColorU32(ImVec4(col_h.x, col_h.y, col_h.z, col_h.w * t_hover));
                
                ImVec2 clipped_min = ImVec2(t_content_start_x - 4.0f, track_bb.Min.y); 
                draw_list->AddRectFilled(clipped_min, track_bb.Max, track_bg_col, 3.0f);
            }

            float t_center_y = track_bb.Min.y + (row_height * 0.5f);
            float t_text_offset_y = t_center_y - (ImGui::GetTextLineHeight() * 0.5f);
            float t_x_cursor = t_content_start_x; 

            // Mini Audio Waveform/Track Icon
            ImU32 track_icon_col = ImGui::GetColorU32(style.Colors[ImGuiCol_TextDisabled]);
            draw_list->AddRectFilled(ImVec2(t_x_cursor, t_center_y - 4.0f), ImVec2(t_x_cursor + 2.0f, t_center_y + 4.0f), track_icon_col);
            draw_list->AddRectFilled(ImVec2(t_x_cursor + 4.0f, t_center_y - 6.0f), ImVec2(t_x_cursor + 6.0f, t_center_y + 6.0f), track_icon_col);
            draw_list->AddRectFilled(ImVec2(t_x_cursor + 8.0f, t_center_y - 2.0f), ImVec2(t_x_cursor + 10.0f, t_center_y + 2.0f), track_icon_col);
            t_x_cursor += 18.0f;

            float max_track_title_width = track_bb.Max.x - t_x_cursor - 8.0f;
            if (!track.duration.empty()) {
                ImVec2 ts = ImGui::CalcTextSize(track.duration.c_str());
                max_track_title_width -= (ts.x + 8.0f);
                draw_list->AddText(ImVec2(track_bb.Max.x - ts.x - 8.0f, t_text_offset_y), ImGui::GetColorU32(ImGuiCol_TextDisabled), track.duration.c_str());
            }

            if (max_track_title_width > 0.0f) {
                ImVec2 track_text_min = ImVec2(t_x_cursor, track_bb.Min.y);
                ImVec2 track_text_max = ImVec2(t_x_cursor + max_track_title_width, track_bb.Max.y);
                
                ImU32 track_text_color = ImGui::GetColorU32(ImVec4(0.13f, 0.13f, 0.13f, 1.0f)); // Re-aligned text to match your light theme text variable state (#212121)
                ImGui::PushStyleColor(ImGuiCol_Text, track_text_color);
                ImGui::RenderTextClipped(track_text_min, track_text_max, track.name.c_str(), nullptr, nullptr, ImVec2(0.0f, 0.5f));
                ImGui::PopStyleColor();
            }

            // ── CONTEXT MENU CONFIGURATION (TWEAK THESE) ──
            // Width adjustments
            float context_menu_width = ImGui::GetFontSize() * 10.5f; 

            // Padding adjustments (Reduced from 12.0f, 10.0f for a tighter look)
            ImVec2 menu_padding = ImVec2(4.0f, 6.0f); 


            // ── COMPLETED CONTEXT MENU FOR PLAYLIST TRACK ENTRIES ──
            // Push our custom tighter padding and item spacing metrics
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, menu_padding);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 6.0f)); 

            // Apply your fine-tuned width constraint here
            ImGui::SetNextWindowSize(ImVec2(context_menu_width, 0.0f)); 

            if (ImGui::BeginPopupContextItem(track_id_str.c_str())) {

                // Helper lambda to draw custom menu items with mixed fonts
                auto DrawIconButtonMenuItem = [](const char* icon, const char* label, bool enabled, ImVec4 text_color = ImGui::GetStyle().Colors[ImGuiCol_Text]) -> bool {
                    bool pressed = false;
                    ImGui::BeginDisabled(!enabled);
                    
                    if (ImGui::Selectable(label, false, ImGuiSelectableFlags_SpanAllColumns)) {
                        pressed = true;
                    }
                    
                    // This dynamically respects whatever WindowPadding.x you push above!
                    ImGui::SameLine(ImGui::GetStyle().WindowPadding.x);
                    ImGui::PushFont(FontManager::icons());
                    if (!enabled) {
                        ImGui::TextDisabled("%s", icon);
                    } else {
                        ImGui::TextColored(text_color, "%s", icon);
                    }
                    ImGui::PopFont();
                    
                    ImGui::EndDisabled();
                    return pressed;
                };

                // 1. Move Track Up Placement Operation
                if (DrawIconButtonMenuItem(ICON_LC_ARROW_UP, "    Move Up", (t > 0))) {
                    // Action: Swap placement index in backend vector structure
                }

                // 2. Move Track Down Placement Operation
                if (DrawIconButtonMenuItem(ICON_LC_ARROW_DOWN, "    Move Down", (t < (int)group.tracks.size() - 1))) {
                    // Action: Swap placement index in backend vector structure
                }

                // 3. Deletion Management Option
                ImVec4 danger_color = ImVec4(0.75f, 0.21f, 0.16f, 1.0f); // #C0362A
                ImGui::PushStyleColor(ImGuiCol_Text, danger_color);
                if (DrawIconButtonMenuItem(ICON_LC_TRASH_2, "    Delete from Playlist", true, danger_color)) {
                    // Action: Remove specific tracking reference node from index array structure
                }
                ImGui::PopStyleColor();

                ImGui::EndPopup();
            }

            ImGui::PopStyleVar(2); // Safely pops WindowPadding and ItemSpacing
        }
    }
}

void StrictTwoTierPlaylistView(const char* str_id, const std::vector<PlaylistGroup>& list)
{
    static ImGuiID global_active_playlist_folder_id = 0;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    for (int i = 0; i < (int)list.size(); i++) {
        RenderPlaylistGroupNode(list[i], str_id, i, global_active_playlist_folder_id);
    }
    ImGui::PopStyleVar();
}