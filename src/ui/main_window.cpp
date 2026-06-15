#include <imgui.h>
#include <algorithm>

#include "ui/main_window.hpp"

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

    // 1. Root canvas setup (Absolute size required here to bootstrap the viewport)
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

    // We keep these two overrides because the root canvas MUST span edge-to-edge
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("##root", nullptr, rootFlags);
    ImGui::PopStyleVar(2);

    // --- GEOMETRY DESIGN VARIABLES ---
    const float leftColW     = 268.0f;
    const float rightBottomH = 130.0f;
    const float tabBarH      = 36.0f;

    // Grab the actual available height of our root canvas dynamically
    const float totalAvailableH = ImGui::GetContentRegionAvail().y;
    const float leftHalfH      = totalAvailableH / 2.0f;

    // Fetch the explicit layout border color line value from your theme style
    ImVec4 borderColor = ImGui::GetStyle().Colors[ImGuiCol_Border]; 

    // --- LEFT COLUMN ---
    // Force padding to 0 so nested edge cards look flush
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
    ImGui::PushStyleColor(ImGuiCol_ChildBg, borderColor);

    // Keep border set to false since the filled background now creates the line natively
    ImGui::BeginChild("##left_col", {leftColW, 0.0f}, false, ImGuiWindowFlags_NoScrollbar);
    
    // Pop layout overrides immediately after establishing the context so inner cards revert to white
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    // Eliminate empty gaps between inner main cards
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
        
        // FEATURE 1: Now Playing / Controller Panel
        ImGui::BeginChild("##now_playing_card", {0.0f, leftHalfH}, true, ImGuiWindowFlags_NoScrollbar);
        // TODO: Implement "Now Playing" artwork, track metadata, and mini playback controls
        ImGui::EndChild();
        
        // FEATURE 2: Tab Panel Parent Container
        // Inject padding and spacing; must remain active until inner children are placed
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 6.0f));

        ImGui::BeginChild("##tab_panel", {0.0f, 0.0f}, true);
            
            // FEATURE 2A: Tab Bar (Navigation Header)
            // Sits precisely 8px away from top, left, and right borders of the container panel
            ImGui::BeginChild("##tab_bar", {0.0f, tabBarH}, true, ImGuiWindowFlags_NoScrollbar);
            // TODO: Implement structural navigation tabs (e.g., Playlists, Queue, History)
            ImGui::EndChild();
            
            // <-- ImGui dynamically injects your 6px vertical ItemSpacing gap directly here!
            
            // FEATURE 2B: Tab Content Viewport
            // Sits precisely 8px away from bottom, left, and right borders of the container panel
            ImGui::BeginChild("##tab_content", {0.0f, 0.0f}, true);
            // TODO: Implement conditional layout swap based on active tab state
            ImGui::EndChild();

        ImGui::EndChild(); // ##tab_panel

        // Safe to pop now that both inner children have been placed
        ImGui::PopStyleVar(2);

    ImGui::PopStyleVar(); // Restore global item spacing

    ImGui::EndChild(); // ##left_col

    ImGui::SameLine(0, 0); // Stitch columns together perfectly edge-to-edge horizontally

    // --- RIGHT COLUMN ---
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
    ImGui::PushStyleColor(ImGuiCol_ChildBg, borderColor);
    ImGui::BeginChild("##right_col", {0.0f, 0.0f}, false, ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
        
        // FEATURE 3: Main Dashboard / Library Explorer
        ImGui::BeginChild("##library", {0.0f, -rightBottomH}, true);
        // TODO: Implement grid view for music albums, track lists, and search queries
        ImGui::EndChild();
        
        // FEATURE 4: Primary Audio Controller & Waveform Visualization
        ImGui::BeginChild("##waveform", {0.0f, 0.0f}, true, ImGuiWindowFlags_NoScrollbar);
        // TODO: Implement interactive audio seek-bar, audio stream waveform visualization, volume sliders
        ImGui::EndChild();

    ImGui::PopStyleVar(); // Restore global item spacing
        
    ImGui::EndChild(); // ##right_col

    ImGui::End(); // ##root
}