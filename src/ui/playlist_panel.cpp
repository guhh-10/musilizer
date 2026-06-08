#include <imgui.h>

#include "ui/playlist_panel.hpp"

PlaylistPanel::PlaylistPanel(Player& player) : player_(player) {}

void PlaylistPanel::draw() {
    ImGui::TextUnformatted("Playlists");
    ImGui::Separator();

    // ── Playlist list ─────────────────────────────────────────────────────────

    const auto& playlists = player_.playlists();

    ImGui::BeginChild("##pllist", {0, -60.0f}, false);

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

    // ── New playlist input ────────────────────────────────────────────────────

    ImGui::Separator();
    ImGui::SetNextItemWidth(-60.0f);
    ImGui::InputTextWithHint("##newpl", "New playlist...",
                              newPlaylistName_, sizeof(newPlaylistName_));
    ImGui::SameLine();
    if (ImGui::Button("Add") && newPlaylistName_[0] != '\0') {
        player_.addPlaylist(Playlist(newPlaylistName_));
        newPlaylistName_[0] = '\0';
    }
}