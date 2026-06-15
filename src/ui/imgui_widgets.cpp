#include <imgui.h>
#include <imgui_internal.h>

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