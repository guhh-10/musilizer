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
    , playlistPanel_(player)
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

    // --- GEOMETRY DESIGN VARIABLES ---
    const float leftColW     = 268.0f;
    const float rightBottomH = 130.0f;
    const float tabBarH      = 36.0f;

    const float totalAvailableH = ImGui::GetContentRegionAvail().y;
    const float leftHalfH      = totalAvailableH / 1.8f;

    ImVec4 borderColor = ImGui::GetStyle().Colors[ImGuiCol_Border]; 

    // --- LEFT COLUMN ---
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
    ImGui::PushStyleColor(ImGuiCol_ChildBg, borderColor);
    ImGui::BeginChild("##left_col", {leftColW, 0.0f}, false, ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

    // FEATURE 1: Now Playing / Controller Panel
    // 1. Control the padding *inside* the master now_playing_card container
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2.0f, 2.0f));
    ImGui::BeginChild("##now_playing_card", {0.0f, leftHalfH}, true, ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleVar(); // Balance the push immediately after BeginChild

    // Calculate half height of the available area inside this child
    float AlbumArtH = ImGui::GetContentRegionAvail().y * 0.65;

    // --- UPPER HALF: Album Art ---
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));
    // 1. Force the child border color to be completely transparent
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); 

    // Keep this as 'true' so structural sizing doesn't break
    ImGui::BeginChild("##album_art_zone", {0.0f, AlbumArtH}, true, ImGuiWindowFlags_NoScrollbar);
    playerPanel_.drawAlbumArt();
    ImGui::EndChild();

    ImGui::PopStyleColor(); // Restore border color immediately
    ImGui::PopStyleVar();   // Balance the album art padding

    ImGui::Dummy(ImVec2(0.0f, 4.0f));

    // --- BOTTOM HALF: Player ---
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f, 4.0f));
    // 2. Force the child border color to be completely transparent here too
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

    // Keep this as 'true' 
    ImGui::BeginChild("##mini_player_zone", {0.0f, 0.0f}, true, ImGuiWindowFlags_NoScrollbar);
    playerPanel_.drawPlayerZone();
    ImGui::EndChild();

    ImGui::PopStyleColor(); // Restore border color immediately
    ImGui::PopStyleVar();   // Balance the player padding

    ImGui::EndChild(); // ##now_playing_card

    // FEATURE 2: Tab Panel Container
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));
    // 1. Change the Y item spacing to 0.0f inside this container so widgets touch perfectly
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   ImVec2(2.0f, 2.0f)); 
    ImGui::BeginChild("##tab_panel", {0.0f, 0.0f}, true);

        // 2. Strip border padding for the tab bar child window
        
        // FEATURE 2A: Tab Bar 
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyle().Colors[ImGuiCol_Header]);

        ImGui::BeginChild("##tab_bar", {0.0f, tabBarH}, true, ImGuiWindowFlags_NoScrollbar);
        tabPanel_.drawTabBar();
        ImGui::EndChild();

        // 2. Pop the color immediately so it doesn't bleed into the Tab Content below
        ImGui::PopStyleColor();

        // FEATURE 2B: Tab Content
        // NOTE: Also changed border flag to 'false' so it blends seamlessly into the parent container
        ImGui::BeginChild("##tab_content", {0.0f, 0.0f}, false);
        tabPanel_.drawTabContent();
        ImGui::EndChild();

    ImGui::EndChild(); // ##tab_panel
    ImGui::PopStyleVar(2); // Restores WindowPadding and ItemSpacing

    ImGui::PopStyleVar(); // Restore global item spacing

    ImGui::EndChild(); // ##left_col

    ImGui::SameLine(0, 0);

    // --- RIGHT COLUMN ---
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
    ImGui::PushStyleColor(ImGuiCol_ChildBg, borderColor);
    ImGui::BeginChild("##right_col", {0.0f, 0.0f}, false, ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

    // FEATURE 3: Library Explorer
    ImGui::BeginChild("##library", {0.0f, -rightBottomH}, true);
    // TODO: grid view for albums, track lists, search queries
    ImGui::EndChild();

    // FEATURE 4: Audio Controller & Waveform
    ImGui::BeginChild("##waveform", {0.0f, 0.0f}, true, ImGuiWindowFlags_NoScrollbar);
    // TODO: seek-bar, waveform visualization, volume sliders
    ImGui::EndChild();

    ImGui::PopStyleVar(); // Restore global item spacing

    ImGui::EndChild(); // ##right_col

    ImGui::End(); // ##root
}