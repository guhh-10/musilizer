#include <imgui.h>

#include "ui/playlist_panel.hpp"

PlaylistPanel::PlaylistPanel(Player& player) : player_(player) {}

void PlaylistPanel::draw() {
    // 1. Core measurements for internal logic
    const float spacingX = ImGui::GetStyle().ItemSpacing.x;
    const float spacingY = ImGui::GetStyle().ItemSpacing.y;
    const float buttonWidth = 60.0f;

    // 2. Draw Title Header
    ImGui::TextUnformatted("Playlists");
    ImGui::Separator();

    // 3. Estimate how much space the bottom input section takes up 
    //    We explicitly add a 20-pixel safety margin to clear layout limits.
    const float footerHeight = ImGui::GetFrameHeight() + spacingY * 2.0f + 20.0f;

    // ── Playlist list ─────────────────────────────────────────────────────────
    const auto& playlists = player_.playlists();

    // Scrollable track child panel sets an explicit safe bottom ceiling
    ImGui::BeginChild("##pllist", ImVec2(0.0f, -footerHeight), false);

    for (int i = 0; i < static_cast<int>(playlists.size()); ++i) {
        const Playlist& pl = playlists[i];

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanFullWidth;
        if (selectedPlaylist_ == i)
            flags |= ImGuiTreeNodeFlags_Selected;

        const bool open = ImGui::TreeNodeEx(pl.getName().c_str(), flags);

        // Single-click selects; double-click plays
        if (ImGui::IsItemClicked(0)) {
            selectedPlaylist_ = i;
            if (ImGui::IsMouseDoubleClicked(0))
                player_.playPlaylist(pl);
        }

        // Right-click context on playlist header
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Play Playlist"))
                player_.playPlaylist(pl);
            if (ImGui::MenuItem("Delete Playlist"))
                player_.removePlaylist(pl.getName());
            ImGui::EndPopup();
        }

        if (open) {
            // Show track names inside the playlist
            for (const fs::path& path : pl.getPlaylistTracks()) {
                ImGui::BulletText("%s", path.filename().string().c_str());
            }
            ImGui::TreePop();
        }
    }

    ImGui::EndChild();

    // ── New playlist input footer ─────────────────────────────────────────────
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 4.0f)); // Small physical vertical spacer line

    // Hardcode an explicit 8-pixel horizontal margin buffer to override your 
    // parent window's borders if it's set to zero.
    const float edgeMargin = 8.0f;

    // Shift the left drawing pen inwards from the left wall manually
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + edgeMargin);

    // Set the input box width: 
    // Available space - button - gap between them - (left border inset + right border inset)
    float calculatedInputWidth = ImGui::GetContentRegionAvail().x - buttonWidth - spacingX - (edgeMargin * 2.0f);
    
    // Safety fallback step: If the window collapses too small, prevent breaking layout numbers
    if (calculatedInputWidth < 50.0f) {
        calculatedInputWidth = 50.0f;
    }
    
    ImGui::SetNextItemWidth(calculatedInputWidth);
    
    // Draw the Input Text
    ImGui::InputTextWithHint("##newpl", "New playlist...",
                             newPlaylistName_, sizeof(newPlaylistName_));

    // Create a helper boolean to check if we should submit the playlist
    bool shouldSubmit = false;

    // Check if the user pressed Enter ONLY while focused inside this specific input bar
    if (ImGui::IsItemFocused()) {
        if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) {
            shouldSubmit = true;
        }
    }

    ImGui::SameLine();
    
    // Draw the button with a matching fixed width
    if (ImGui::Button("Add", ImVec2(buttonWidth, 0.0f))) {
        shouldSubmit = true;
    }

    // Execute the addition if either action triggered it and the string isn't empty
    if (shouldSubmit && newPlaylistName_[0] != '\0') {
        player_.addPlaylist(Playlist(newPlaylistName_));
        newPlaylistName_[0] = '\0'; // Clear the input box
        
        // Re-focus the text box automatically so they can type another playlist immediately
        ImGui::SetKeyboardFocusHere(-1); 
    }
}