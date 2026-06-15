#include <imgui.h>

#include "ui/imgui_style.hpp"

// Warm paper-white light theme with terracotta accent (#D1703D)
void SetupMusicPlayerLightStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // --- Layout / shape ---
    style.WindowRounding    = 14.0f;
    style.ChildRounding     = 8.0f;
    style.FrameRounding     = 7.0f;
    style.PopupRounding     = 8.0f;
    style.ScrollbarRounding = 99.0f; 
    style.GrabRounding      = 99.0f; 
    style.TabRounding       = 6.0f;

    style.WindowPadding    = ImVec2(18.0f, 18.0f);
    style.FramePadding     = ImVec2(12.0f, 8.0f);
    style.ItemSpacing      = ImVec2(8.0f,  8.0f);
    style.ItemInnerSpacing = ImVec2(6.0f,  6.0f);
    style.IndentSpacing    = 18.0f;
    style.ScrollbarSize    = 4.0f;  
    style.GrabMinSize      = 12.0f;

    style.WindowBorderSize = 0.0f;  
    style.FrameBorderSize  = 1.0f;  
    style.PopupBorderSize  = 1.0f;
    style.ChildBorderSize  = 1.0f;

    // --- Palette ---
    ImVec4 surface    = ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // #FFFFFF
    ImVec4 surface2   = ImVec4(0.96f, 0.95f, 0.94f, 1.00f); // #F5F3EF
    ImVec4 border     = ImVec4(0.91f, 0.90f, 0.88f, 1.00f); // #E8E6E0
    ImVec4 text       = ImVec4(0.13f, 0.13f, 0.13f, 1.00f); // #212121 
    ImVec4 darkButton = ImVec4(0.13f, 0.13f, 0.13f, 1.00f); // #212121 (RENAMED: Main button fill state)
    ImVec4 muted      = ImVec4(0.55f, 0.55f, 0.55f, 1.00f); // #8C8C8C
    ImVec4 accent     = ImVec4(0.82f, 0.44f, 0.24f, 1.00f); // #D1703D
    ImVec4 accentHov  = ImVec4(0.91f, 0.55f, 0.35f, 1.00f); // #E88C59
    ImVec4 accentAct  = ImVec4(0.71f, 0.35f, 0.16f, 1.00f); // Slightly deeper clay
    ImVec4 accentBg   = ImVec4(0.97f, 0.93f, 0.91f, 1.00f); // #F7EDE8
    ImVec4 danger     = ImVec4(0.75f, 0.21f, 0.16f, 1.00f); // #C0362A

    colors[ImGuiCol_Text]                 = text;
    colors[ImGuiCol_TextDisabled]         = muted;

    // Window / child backgrounds
    colors[ImGuiCol_WindowBg]             = border;       
    colors[ImGuiCol_ChildBg]              = surface;  
    colors[ImGuiCol_PopupBg]              = surface;

    colors[ImGuiCol_Border]               = border;
    colors[ImGuiCol_BorderShadow]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Frame backgrounds
    colors[ImGuiCol_FrameBg]              = surface2;
    colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.92f, 0.91f, 0.89f, 1.00f); 
    colors[ImGuiCol_FrameBgActive]        = surface;                            

    // Title bar settings
    colors[ImGuiCol_TitleBg]              = surface;
    colors[ImGuiCol_TitleBgActive]        = surface;
    colors[ImGuiCol_TitleBgCollapsed]     = surface2;

    colors[ImGuiCol_MenuBarBg]            = surface;

    // Scrollbar styling
    colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_ScrollbarGrab]        = border;
    colors[ImGuiCol_ScrollbarGrabHovered] = muted;
    colors[ImGuiCol_ScrollbarGrabActive]  = accent;

    // Interactive UI controls 
    colors[ImGuiCol_CheckMark]            = accent;
    colors[ImGuiCol_SliderGrab]           = darkButton; // Re-mapped to darkButton preserve variable
    colors[ImGuiCol_SliderGrabActive]     = accent;

    // Buttons (CORRECTED: Remapped to use darkButton states with white text helper approach)
    colors[ImGuiCol_Button]               = muted; // Main flat dark appearance
    colors[ImGuiCol_ButtonHovered]        = ImVec4(0.20f, 0.20f, 0.20f, 1.00f); // Slightly lighter gray-black hover
    colors[ImGuiCol_ButtonActive]         = ImVec4(0.10f, 0.10f, 0.10f, 1.00f); // Deep active press state

    // Table rows / Track Selection Lists
    colors[ImGuiCol_Header]               = accentBg;  
    colors[ImGuiCol_HeaderHovered]        = surface2;  
    colors[ImGuiCol_HeaderActive]         = accentBg;

    // Structural Layout Separators
    colors[ImGuiCol_Separator]            = border;
    colors[ImGuiCol_SeparatorHovered]     = accent;
    colors[ImGuiCol_SeparatorActive]      = accentAct;

    colors[ImGuiCol_ResizeGrip]           = border;
    colors[ImGuiCol_ResizeGripHovered]    = accent;
    colors[ImGuiCol_ResizeGripActive]     = accentAct;

    // Tab Headers
    colors[ImGuiCol_Tab]                  = surface;
    colors[ImGuiCol_TabHovered]           = accentBg;
    colors[ImGuiCol_TabActive]            = accentBg;
    colors[ImGuiCol_TabUnfocused]         = surface;
    colors[ImGuiCol_TabUnfocusedActive]   = surface2;

    // Music Waveform plots
    colors[ImGuiCol_PlotLines]            = accent;
    colors[ImGuiCol_PlotLinesHovered]     = accentHov;
    colors[ImGuiCol_PlotHistogram]        = accent;
    colors[ImGuiCol_PlotHistogramHovered] = accentHov;

    // Selection fields
    colors[ImGuiCol_TextSelectedBg]       = ImVec4(0.82f, 0.44f, 0.24f, 0.25f); 

    // Background mask dimmer
    colors[ImGuiCol_ModalWindowDimBg]     = ImVec4(0.08f, 0.08f, 0.08f, 0.40f);

    (void)danger;
}