#include <imgui.h>

#include "ui/imgui_style.hpp"

void SetupModernDarkStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // --- Layout / shape ---
    style.WindowRounding    = 8.0f;
    style.ChildRounding     = 6.0f;
    style.FrameRounding     = 10.0f;
    style.PopupRounding     = 6.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabRounding      = 6.0f;
    style.TabRounding       = 6.0f;

    style.WindowPadding   = ImVec2(12, 12);
    style.FramePadding    = ImVec2(10, 6);
    style.ItemSpacing     = ImVec2(6, 6);
    style.ItemInnerSpacing= ImVec2(6, 6);
    style.IndentSpacing   = 18.0f;
    style.ScrollbarSize   = 14.0f;
    style.GrabMinSize     = 10.0f;

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize  = 0.0f;
    style.PopupBorderSize  = 1.0f;

    // --- Palette ---
    ImVec4 bg          = ImVec4(0.10f, 0.10f, 0.13f, 1.00f);
    ImVec4 bgLight     = ImVec4(0.14f, 0.14f, 0.18f, 1.00f);
    ImVec4 panel       = ImVec4(0.16f, 0.16f, 0.21f, 1.00f);
    ImVec4 accent      = ImVec4(0.36f, 0.55f, 0.95f, 1.00f); // accent blue
    ImVec4 accentHover = ImVec4(0.46f, 0.65f, 1.00f, 1.00f);
    ImVec4 accentActive= ImVec4(0.30f, 0.46f, 0.85f, 1.00f);
    ImVec4 text        = ImVec4(0.92f, 0.92f, 0.95f, 1.00f);
    ImVec4 textDim     = ImVec4(0.55f, 0.55f, 0.60f, 1.00f);

    colors[ImGuiCol_Text]                 = text;
    colors[ImGuiCol_TextDisabled]         = textDim;
    colors[ImGuiCol_WindowBg]             = bg;
    colors[ImGuiCol_ChildBg]              = ImVec4(0,0,0,0);
    colors[ImGuiCol_PopupBg]              = bgLight;
    colors[ImGuiCol_Border]               = ImVec4(0.25f, 0.25f, 0.30f, 0.50f);

    colors[ImGuiCol_FrameBg]              = panel;
    colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.20f, 0.20f, 0.26f, 1.00f);
    colors[ImGuiCol_FrameBgActive]        = ImVec4(0.24f, 0.24f, 0.30f, 1.00f);

    colors[ImGuiCol_TitleBg]              = bgLight;
    colors[ImGuiCol_TitleBgActive]        = bgLight;
    colors[ImGuiCol_TitleBgCollapsed]     = bgLight;

    colors[ImGuiCol_MenuBarBg]            = bgLight;

    colors[ImGuiCol_ScrollbarBg]          = bg;
    colors[ImGuiCol_ScrollbarGrab]        = panel;
    colors[ImGuiCol_ScrollbarGrabHovered] = accentHover;
    colors[ImGuiCol_ScrollbarGrabActive]  = accentActive;

    colors[ImGuiCol_CheckMark]            = accent;
    colors[ImGuiCol_SliderGrab]           = accent;
    colors[ImGuiCol_SliderGrabActive]     = accentActive;

    colors[ImGuiCol_Button]               = panel;
    colors[ImGuiCol_ButtonHovered]        = accentHover;
    colors[ImGuiCol_ButtonActive]         = accentActive;

    colors[ImGuiCol_Header]               = accent;
    colors[ImGuiCol_HeaderHovered]        = accentHover;
    colors[ImGuiCol_HeaderActive]         = accentActive;

    colors[ImGuiCol_Separator]            = ImVec4(0.25f, 0.25f, 0.30f, 0.50f);
    colors[ImGuiCol_SeparatorHovered]     = accent;
    colors[ImGuiCol_SeparatorActive]      = accentActive;

    colors[ImGuiCol_ResizeGrip]           = accent;
    colors[ImGuiCol_ResizeGripHovered]    = accentHover;
    colors[ImGuiCol_ResizeGripActive]     = accentActive;

    colors[ImGuiCol_Tab]                  = bgLight;
    colors[ImGuiCol_TabHovered]           = accentHover;
    colors[ImGuiCol_TabActive]            = accent;
    colors[ImGuiCol_TabUnfocused]         = bgLight;
    colors[ImGuiCol_TabUnfocusedActive]   = panel;

    colors[ImGuiCol_PlotLines]            = accent;
    colors[ImGuiCol_PlotHistogram]        = accent;

    colors[ImGuiCol_TextSelectedBg]       = ImVec4(0.36f, 0.55f, 0.95f, 0.35f);
}