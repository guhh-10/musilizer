#include <imgui.h>

#include "ui/imgui_style.hpp"

// Warm paper-white light theme with terracotta accent (#C0622A)
void SetupMusicPlayerLightStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // --- Layout / shape ---
    // Matches: border-radius values from .app (14px), .album-art (8px),
    //          .btn (6px), .ctx-menu (8px), .search-box (7px)
    style.WindowRounding    = 8.0f;
    style.ChildRounding     = 6.0f;
    style.FrameRounding     = 6.0f;
    style.PopupRounding     = 8.0f;
    style.ScrollbarRounding = 99.0f; // pill-shaped, matches scrollbar-thumb border-radius:99px
    style.GrabRounding      = 6.0f;
    style.TabRounding       = 6.0f;

    // Matches: WindowPadding ~ .app / card padding (18px 16px), FramePadding ~ .pl-add-row input (6px 8px)
    style.WindowPadding    = ImVec2(16.0f, 16.0f);
    style.FramePadding     = ImVec2(8.0f,  6.0f);
    style.ItemSpacing      = ImVec2(6.0f,  6.0f);
    style.ItemInnerSpacing = ImVec2(6.0f,  6.0f);
    style.IndentSpacing    = 18.0f;
    style.ScrollbarSize    = 3.0f;  // matches .queue-list::-webkit-scrollbar { width: 3px }
    style.GrabMinSize      = 10.0f;

    style.WindowBorderSize = 0.5f;  // matches border: 0.5px solid var(--border)
    style.FrameBorderSize  = 0.5f;
    style.PopupBorderSize  = 0.5f;

    // --- Palette ---
    // --bg:         #F5F3EF  warm off-white page background
    // --surface:    #FFFFFF  sidebar / cards / search box
    // --surface2:   #EDEBE6  secondary surface, hover states, frame bg
    // --border:     rgba(0,0,0,0.07)
    // --text:       #1A1917  near-black body text
    // --muted:      #8A8780  secondary / disabled text, icons
    // --accent:     #C0622A  terracotta — primary interactive color
    // --accent2:    #E8855A  lighter terracotta — hover state
    // --accent-bg:  #FBF0EB  pale terracotta tint — selected row / text selection bg

    ImVec4 bg         = ImVec4(0.96f, 0.95f, 0.94f, 1.00f); // #F5F3EF
    ImVec4 surface    = ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // #FFFFFF
    ImVec4 surface2   = ImVec4(0.93f, 0.92f, 0.90f, 1.00f); // #EDEBE6
    ImVec4 border     = ImVec4(0.00f, 0.00f, 0.00f, 0.07f); // rgba(0,0,0,0.07)
    ImVec4 text       = ImVec4(0.10f, 0.10f, 0.09f, 1.00f); // #1A1917
    ImVec4 muted      = ImVec4(0.54f, 0.53f, 0.50f, 1.00f); // #8A8780
    ImVec4 accent     = ImVec4(0.75f, 0.38f, 0.16f, 1.00f); // #C0622A
    ImVec4 accentHov  = ImVec4(0.91f, 0.52f, 0.35f, 1.00f); // #E8855A
    ImVec4 accentAct  = ImVec4(0.62f, 0.30f, 0.10f, 1.00f); // slightly darker than accent
    ImVec4 accentBg   = ImVec4(0.98f, 0.94f, 0.92f, 1.00f); // #FBF0EB
    ImVec4 danger     = ImVec4(0.75f, 0.21f, 0.16f, 1.00f); // #C0362A (.ctx-item.danger)

    colors[ImGuiCol_Text]                 = text;
    colors[ImGuiCol_TextDisabled]         = muted;

    // Window / child backgrounds
    colors[ImGuiCol_WindowBg]             = bg;       // --bg
    colors[ImGuiCol_ChildBg]              = surface;  // --surface (sidebar, cards)
    colors[ImGuiCol_PopupBg]              = surface;  // --surface (.ctx-menu background)

    colors[ImGuiCol_Border]               = border;
    colors[ImGuiCol_BorderShadow]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Frame backgrounds (.pl-add-row input, .search-box → --surface2)
    colors[ImGuiCol_FrameBg]              = surface2;
    colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.90f, 0.89f, 0.87f, 1.00f); // surface2 slightly lighter
    colors[ImGuiCol_FrameBgActive]        = accentBg; // focused input gets accent-bg tint

    // Title bar (maps to --surface / sidebar header area)
    colors[ImGuiCol_TitleBg]              = surface;
    colors[ImGuiCol_TitleBgActive]        = surface;
    colors[ImGuiCol_TitleBgCollapsed]     = bg;

    colors[ImGuiCol_MenuBarBg]            = surface;

    // Scrollbar (3px pill, near-invisible; thumb is --border color)
    colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_ScrollbarGrab]        = border;
    colors[ImGuiCol_ScrollbarGrabHovered] = muted;
    colors[ImGuiCol_ScrollbarGrabActive]  = accent;

    // Controls — accent color (accent-color: var(--accent) on range inputs)
    colors[ImGuiCol_CheckMark]            = accent;
    colors[ImGuiCol_SliderGrab]           = accent;   // seek / volume sliders
    colors[ImGuiCol_SliderGrabActive]     = accentAct;

    // Buttons (.btn-play background: --accent, hover: --accent2)
    colors[ImGuiCol_Button]               = surface2; // plain .btn has no background until hover
    colors[ImGuiCol_ButtonHovered]        = accentHov;
    colors[ImGuiCol_ButtonActive]         = accentAct;

    // Selectable / header rows (.trow.playing → accent-bg; selected text → accent)
    colors[ImGuiCol_Header]               = accentBg; // .pl-header.selected, .trow.playing
    colors[ImGuiCol_HeaderHovered]        = surface2; // .q-item:hover, .trow:hover → --surface2
    colors[ImGuiCol_HeaderActive]         = accentBg;

    // Separators (.ctx-sep, th-row border → --border)
    colors[ImGuiCol_Separator]            = border;
    colors[ImGuiCol_SeparatorHovered]     = accent;
    colors[ImGuiCol_SeparatorActive]      = accentAct;

    colors[ImGuiCol_ResizeGrip]           = border;
    colors[ImGuiCol_ResizeGripHovered]    = accent;
    colors[ImGuiCol_ResizeGripActive]     = accentAct;

    // Tabs (.tab active → accent; inactive → muted text on --surface)
    colors[ImGuiCol_Tab]                  = surface;
    colors[ImGuiCol_TabHovered]           = accentBg;
    colors[ImGuiCol_TabActive]            = accentBg; // active tab gets accent underline + accent text
    colors[ImGuiCol_TabUnfocused]         = surface;
    colors[ImGuiCol_TabUnfocusedActive]   = surface2;

    // Plot / waveform (cx.strokeStyle = '#C0622A' → accent)
    colors[ImGuiCol_PlotLines]            = accent;
    colors[ImGuiCol_PlotLinesHovered]     = accentHov;
    colors[ImGuiCol_PlotHistogram]        = accent;
    colors[ImGuiCol_PlotHistogramHovered] = accentHov;

    // Text selection (--accent at low alpha, matches .TextSelectedBg intent)
    colors[ImGuiCol_TextSelectedBg]       = ImVec4(0.75f, 0.38f, 0.16f, 0.20f); // accent @ 20%

    // Modal dimming — subtle, suits a light UI
    colors[ImGuiCol_ModalWindowDimBg]     = ImVec4(0.10f, 0.10f, 0.09f, 0.35f);

    // --- Convenience note ---
    // To highlight a "danger" action (equivalent to .ctx-item.danger { color: #C0362A }),
    // push the danger color before drawing the widget:
    //   ImGui::PushStyleColor(ImGuiCol_Text, danger);
    //   ImGui::Selectable("Delete playlist");
    //   ImGui::PopStyleColor();
    (void)danger; // suppress unused-variable warning if not used inline
}