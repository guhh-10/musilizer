#include "imgui.h"
#include "imgui_internal.h"

#include "ui/tab_panel.hpp"

static ImFont* resolveFont(ButtonFont font)
{
    switch (font) {
        case ButtonFont::Bold:       return FontManager::bold();
        case ButtonFont::Icons:      return FontManager::icons();
        case ButtonFont::LargeIcons: return FontManager::largeIcons();
        case ButtonFont::Regular:
        default:                     return FontManager::regular();
    }
}

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
    
    // --- MODIFIED MATH ---
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

    // --- CRITICAL FIX FOR CLIP RECT ---
    // We pass visual_bb instead of bb so text properly center-aligns inside the animated shrinking/growing box
    ImGui::RenderTextClipped(
        visual_bb.Min, visual_bb.Max,
        label, NULL, &label_size, style.ButtonTextAlign, &visual_bb
    );

    ImGui::PopFont();

    return pressed;
}

void TabPanel::drawTabBar()
{
    // Strip frame rounding and padding so buttons touch the borders seamlessly
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));

    const ImGuiStyle& style = ImGui::GetStyle();
    const float spacing = style.ItemSpacing.x;
    
    // FIX: Calculate total static width based on the current window size footprint
    // This stops GetContentRegionAvail() from fluctuating during the size animation loop
    float totalWidth = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
    const float buttonW = (totalWidth - spacing) / 2.0f; 
    
    const float buttonH = ImGui::GetContentRegionAvail().y; 

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 1.00f, 1.00f, 1.00f));

        if (SmoothRadioButton("Playlists", {buttonW, buttonH}, ButtonFont::Bold, m_activeTab == 0))
            m_activeTab = 0;
            
        ImGui::SameLine(0, spacing);
        
        if (SmoothRadioButton("Up Next", {buttonW, buttonH}, ButtonFont::Bold, m_activeTab == 1))
            m_activeTab = 1;
            
    ImGui::PopStyleColor();

    ImGui::PopStyleVar(); // Restore global rounding and padding
}

void TabPanel::drawTabContent()
{
    // TODO: Implement conditional layout swap based on active tab state
}