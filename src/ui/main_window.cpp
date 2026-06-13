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

void MainWindow::draw() {
    ImGuiIO& io = ImGui::GetIO();
    
    // Enforce minimum window size
    const float minWindowW = 800.0f;
    const float minWindowH = 600.0f;
    
    const float totalW = std::max(io.DisplaySize.x, minWindowW);
    const float totalH = std::max(io.DisplaySize.y, minWindowH);

    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize({totalW, totalH});
    ImGui::SetNextWindowBgAlpha(1.0f);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar    |
        ImGuiWindowFlags_NoResize      |
        ImGuiWindowFlags_NoMove        |
        ImGuiWindowFlags_NoScrollbar   |
        ImGuiWindowFlags_NoCollapse    |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("##root", nullptr, flags);
    ImGui::PopStyleVar(2);

    const float playerBarH  = 90.0f;
    const float contentH    = totalH - playerBarH;
    
    // Adjusted: Playlist is now 15% of width instead of 20%, 
    // giving more X-axis space to the Library.
    const float playlistW   = std::max(180.0f, totalW * 0.15f); 
    const float libraryW    = totalW - playlistW;

    // ── Playlist panel (Now on the Left Side) ──────────────────────────

    // Start at the very left edge (X = 0)
    ImGui::SetNextWindowPos({0, 0}); 
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
    ImGui::BeginChild("##playlists", {playlistW, contentH}, false);
    playlistPanel_.draw();
    ImGui::EndChild();
    ImGui::PopStyleColor();

    // ── Library panel (Now on the Right Side) ───────────────────────────────

    // Shift the cursor to the right, past the playlist width
    ImGui::SetCursorPos({playlistW, 0}); 
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {8, 8});
    ImGui::BeginChild("##library", {libraryW, contentH}, false);
    libraryPanel_.draw();
    ImGui::EndChild();
    ImGui::PopStyleVar();

    // ── Player bar ────────────────────────────────────────────────────────────

    ImGui::SetCursorPos({0, contentH});
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.10f, 0.10f, 0.10f, 1.0f));
    ImGui::BeginChild("##player", {totalW, playerBarH}, false);
    playerPanel_.draw();
    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::End();
}