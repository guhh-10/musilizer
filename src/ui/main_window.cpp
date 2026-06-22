#include <imgui.h>
#include <algorithm>

#include "ui/main_window.hpp"
#include "ui/tab_panel.hpp"
#include "ui/player_panel.hpp"

MainWindow::MainWindow(Player& player, SearchController& search)
    : player_(player)
    , search_(search)
    , playerPanel_(player)
    , libraryPanel_(player, search)
    , tabPanel_(player)
    , waveformPanel_(player)
{}

void MainWindow::draw()
{
    ImGuiIO& io = ImGui::GetIO();

    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::SetNextWindowBgAlpha(1.0f);

    constexpr ImGuiWindowFlags rootFlags =
        ImGuiWindowFlags_NoTitleBar          |
        ImGuiWindowFlags_NoResize            |
        ImGuiWindowFlags_NoMove              |
        ImGuiWindowFlags_NoScrollbar         |
        ImGuiWindowFlags_NoCollapse          |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("##root", nullptr, rootFlags);
    ImGui::PopStyleVar(2);

    const float leftColW      = 268.0f;
    const float tabBarH       = 36.0f;
    const float totalH        = ImGui::GetContentRegionAvail().y;
    const float leftHalfH     = totalH / 1.8f;
    const float rightBottomH  = totalH * 0.30f;

    ImVec4 borderColor = ImGui::GetStyle().Colors[ImGuiCol_Border];

    // ── LEFT COLUMN ──────────────────────────────────────────────────────────
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
    ImGui::PushStyleColor(ImGuiCol_ChildBg, borderColor);
    ImGui::BeginChild("##left_col", {leftColW, 0.0f}, false, ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

    // Now-playing card
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2.0f, 2.0f));
    ImGui::BeginChild("##now_playing_card", {0.0f, leftHalfH}, true, ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleVar();

    float albumH = ImGui::GetContentRegionAvail().y * 0.65f;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0,0,0,0));
    ImGui::BeginChild("##album_art_zone", {0.0f, albumH}, true, ImGuiWindowFlags_NoScrollbar);
    playerPanel_.drawAlbumArt();
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    ImGui::Dummy(ImVec2(0.0f, 4.0f));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f, 4.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0,0,0,0));
    ImGui::BeginChild("##mini_player_zone", {0.0f, 0.0f}, true, ImGuiWindowFlags_NoScrollbar);
    playerPanel_.drawPlayerZone();
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    ImGui::EndChild(); // ##now_playing_card

    // Tab panel
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   ImVec2(2.0f, 2.0f));
    ImGui::BeginChild("##tab_panel", {0.0f, 0.0f}, true);

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyle().Colors[ImGuiCol_Header]);
        ImGui::BeginChild("##tab_bar", {0.0f, tabBarH}, true, ImGuiWindowFlags_NoScrollbar);
        tabPanel_.drawTabBar();
        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::BeginChild("##tab_content", {0.0f, 0.0f}, false);
        ImGui::PushItemFlag(ImGuiItemFlags_NoNav, true);
        tabPanel_.drawTabContent();
        ImGui::PopItemFlag();
        ImGui::EndChild();

    ImGui::EndChild(); // ##tab_panel
    ImGui::PopStyleVar(2);

    ImGui::PopStyleVar(); // global item spacing

    ImGui::EndChild(); // ##left_col

    ImGui::SameLine(0, 0);

    // ── RIGHT COLUMN ─────────────────────────────────────────────────────────
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
    ImGui::PushStyleColor(ImGuiCol_ChildBg, borderColor);
    ImGui::BeginChild("##right_col", {0.0f, 0.0f}, false, ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   ImVec2(0.0f, 0.0f));

    // Library
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 4.0f));
    ImGui::BeginChild("##library", {0.0f, -rightBottomH}, true);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    {
        ImGuiStyle& style   = ImGui::GetStyle();
        const float padding = 24.0f;

        float titleH  = ImGui::GetTextLineHeightWithSpacing();
        float inputH  = ImGui::GetTextLineHeight() + style.FramePadding.y * 2.0f;
        float searchH = titleH + inputH + 12.0f + 12.0f;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, style.Colors[ImGuiCol_Border]);
        
        if (ImGui::BeginChild("##search_header", ImVec2(0.0f, searchH), false, ImGuiWindowFlags_NoScrollbar)) {
            libraryPanel_.drawSearchBar(padding);
        }
        ImGui::EndChild();
        
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();

        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   ImVec2(0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, style.Colors[ImGuiCol_Border]);
        
        if (ImGui::BeginChild("##library_table_container", ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_NoScrollbar)) {
            ImGui::PushItemFlag(ImGuiItemFlags_NoNav, true);
            libraryPanel_.drawTableTrack();
            ImGui::PopItemFlag();
        }
        ImGui::EndChild();
        
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
    }
    ImGui::EndChild(); // ##library

    // Waveform / seek placeholder
    ImGui::BeginChild("##waveform", {0.0f, 0.0f}, true, ImGuiWindowFlags_NoScrollbar);
    waveformPanel_.draw();
    ImGui::EndChild();

    ImGui::PopStyleVar(2);
    ImGui::EndChild(); // ##right_col

    ImGui::End(); // ##root
}