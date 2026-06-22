#include <imgui.h>
#include <imgui_internal.h>
#include <IconsLucide.h>

#include "ui/imgui_widgets.hpp"
#include "ui/fonts.hpp"
#include "controller/player.hpp"

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
 
            // --- Step 1: Run Column 1 First to establish the dynamic Row Height ---
            ImGui::TableSetColumnIndex(1);
            float row_top_y = ImGui::GetCursorPosY();

            // 1. Title Text
            ImGui::TextUnformatted(items[i].title.c_str()); //
 
            // 2. Artist Subtitle Text
            ImGui::PushFont(FontManager::qArtist()); //
            ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2.0f); 
            ImGui::TextUnformatted(items[i].artist.c_str());
            ImGui::PopStyleColor();
            ImGui::PopFont();

            // Capture final bounds
            float row_bottom_y = ImGui::GetCursorPosY();
            float content_height = row_bottom_y - row_top_y;
            
            // Calculate vertical center alignment offsets
            float single_line_height = ImGui::GetFontSize();
            float vertical_offset = (content_height - single_line_height) * 0.5f;

            // --- Step 2: Place the Selectable across the full expanded row height ---
            ImGui::TableSetColumnIndex(0);
            
            // Push transparent colors so the Selectable draws no raw default rectangle
            ImGui::PushStyleColor(ImGuiCol_Header,        IM_COL32(0,0,0,0)); //
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(0,0,0,0)); //
            ImGui::PushStyleColor(ImGuiCol_HeaderActive,  IM_COL32(0,0,0,0)); //
 
            std::string selectable_id = "##row_sel_" + std::to_string(i);
            bool is_selected = (out_selected_index && *out_selected_index == i);
 
            // Crucial: Set cursor back to the row top so the hitbox fills from top to bottom
            ImGui::SetCursorPosY(row_top_y);
            ImGui::PushItemFlag(ImGuiItemFlags_NoNav, true);

            // Create an invisible button item matching the exact evaluated height of the row content
            if (ImGui::Selectable(selectable_id.c_str(), is_selected, ImGuiSelectableFlags_SpanAllColumns, ImVec2(0, content_height))) {
                if (out_selected_index) *out_selected_index = i;
                any_clicked = true;
            }
            ImGui::PopItemFlag();
 
            bool row_hovered = ImGui::IsItemHovered();
            ImGui::PopStyleColor(3);
 
            // --- Step 3: Draw Column Content Overlays ---

            // --- Column 0: Center-aligned Number text ---
            ImGui::SameLine(); //
            if (vertical_offset > 0.0f) {
                ImGui::SetCursorPosY(row_top_y + vertical_offset);
            }

            float text_width = ImGui::CalcTextSize(items[i].number.c_str()).x; //
            float column_width = ImGui::GetContentRegionAvail().x; //
            if (text_width < column_width) //
            {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (column_width - text_width) * 0.5f); //
            }
            
            ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]); //
            ImGui::TextUnformatted(items[i].number.c_str()); //
            ImGui::PopStyleColor(); //
 
            // --- Column 2: Duration text ---
            ImGui::TableSetColumnIndex(2);
            if (vertical_offset > 0.0f) {
                ImGui::SetCursorPosY(row_top_y + vertical_offset);
            }
            
            ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]); //
            ImGui::TextUnformatted(items[i].duration.c_str()); //
            ImGui::PopStyleColor(); //

            // Step cursor past the multi-line block for the next row loop pass
            ImGui::SetCursorPosY(row_bottom_y);

            // --- Smooth hover interpolation ---
            if (row_hovered) {
                t += g.IO.DeltaTime * animation_speed;
                if (t > 1.0f) t = 1.0f;
            } else {
                t -= g.IO.DeltaTime * animation_speed;
                if (t < 0.0f) t = 0.0f;
            }
 
            // --- Draw background highlight channel ---
            if (t > 0.01f) { //
                ImGuiTable* table = g.CurrentTable; //
                const float rounding = 6.0f; //
                const float inset_x  = 4.0f; //
                const float inset_y  = 2.0f; //
 
                ImVec2 min_p = ImVec2(table->WorkRect.Min.x + inset_x, table->RowPosY1 + inset_y); //
                ImVec2 max_p = ImVec2(table->WorkRect.Max.x - inset_x, table->RowPosY2 - inset_y); //
 
                ImVec4 col_bg = style.Colors[ImGuiCol_HeaderHovered]; //
                ImU32 row_bg_color = ImGui::GetColorU32(ImVec4(col_bg.x, col_bg.y, col_bg.z, col_bg.w * t)); //
 
                ImGui::TablePushBackgroundChannel(); //
                window->DrawList->AddRectFilled(min_p, max_p, row_bg_color, rounding); //
                ImGui::TablePopBackgroundChannel(); //
            }
        }
 
        ImGui::EndTable(); //
    }
 
    ImGui::PopStyleVar(); //
 
    return any_clicked; //
}

