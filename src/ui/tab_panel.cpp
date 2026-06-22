#include <imgui.h>
#include <imgui_internal.h>
#include <vector>
#include <string>
#include <cstdio>

#include "ui/tab_panel.hpp"
#include "ui/imgui_widgets.hpp"
#include "ui/playlist_tree_view.hpp"
#include "ui/queue_table_view.hpp"
#include "utils/time_format.hpp"

// ── TabPanel ──────────────────────────────────────────────────────────────────

TabPanel::TabPanel(Player& player) : player_(player) {}

void TabPanel::drawTabBar()
{
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));

    const ImGuiStyle& style = ImGui::GetStyle();
    const float spacing = style.ItemSpacing.x;

    float totalWidth = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
    const float buttonW = (totalWidth - spacing) / 2.0f;
    const float buttonH = ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y;

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 1.00f, 1.00f, 1.00f));

    if (SmoothRadioButton("Playlists", {buttonW, buttonH}, ButtonFont::Bold,
                          m_activeTab == ContextView::VIEW_PLAYLIST))
        m_activeTab = ContextView::VIEW_PLAYLIST;

    ImGui::SameLine(0, spacing);

    if (SmoothRadioButton("Up Next", {buttonW, buttonH}, ButtonFont::Bold,
                          m_activeTab == ContextView::VIEW_QUEUE))
        m_activeTab = ContextView::VIEW_QUEUE;

    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

void TabPanel::drawTabContent()
{
    // Push flag to ignore keyboard tab-focus and keyboard navigation indexing inside our tabs
    ImGui::PushItemFlag(ImGuiItemFlags_NoTabStop, true);

    if (m_activeTab == ContextView::VIEW_PLAYLIST)
        drawPlaylistContent();
    else
        drawQueueContent();

    ImGui::PopItemFlag();
}

// ── Playlist tab ──────────────────────────────────────────────────────────────

void TabPanel::drawPlaylistContent()
{
    const auto& playlists = player_.playlists();

    std::vector<PlaylistGroup> groups;
    groups.reserve(playlists.size());

    for (std::size_t playlistIndex = 0; playlistIndex < playlists.size(); ++playlistIndex) {
        const Playlist& pl = playlists[playlistIndex];
        PlaylistGroup g;
        g.name = pl.getName();
        const auto& playlistTracks = pl.getPlaylistTracks();
        for (std::size_t trackIndex = 0; trackIndex < playlistTracks.size(); ++trackIndex) {
            const fs::path& p = playlistTracks[trackIndex];
            PlaylistItemTrack item;
            item.name = p.stem().string();
            item.duration = "";
            g.tracks.push_back(std::move(item));
        }
        groups.push_back(std::move(g));
    }

    PlaylistViewActions actions;
    actions.playPlaylist = [this](int playlistIndex) {
        const auto& playlists = player_.playlists();
        if (playlistIndex >= 0 && playlistIndex < (int)playlists.size()) {
            player_.playPlaylist(playlists[playlistIndex]);
        }
    };
    actions.playTrack = [this](int playlistIndex, int trackIndex) {
        const auto& playlists = player_.playlists();
        if (playlistIndex >= 0 && playlistIndex < (int)playlists.size()) {
            player_.playPlaylistStartingAt(playlists[playlistIndex], trackIndex);
        }
    };
    actions.removePlaylist = [this](const std::string& name) {
        player_.removePlaylist(name);
    };
    actions.moveTrack = [this](const std::string& name, int from, int to) {
        player_.moveTrackInPlaylist(name, from, to);
    };
    actions.removeTrack = [this](const std::string& name, const fs::path& path) {
        player_.removeTrackFromPlaylist(name, path);
    };
    actions.getPlaylists = [this]() -> const std::vector<Playlist>& {
        return player_.playlists();
    };

    ImGui::BeginChild("##tab_pl_scroll", ImVec2(0.0f, 0.0f), false);

    if (groups.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        ImGui::TextUnformatted("No playlists yet.");
        ImGui::PopStyleColor();
    } else {
        StrictTwoTierPlaylistView("##pl_tree", groups, actions);
    }

    ImGui::EndChild();
}

void TabPanel::drawQueueContent()
{
    const std::vector<const Track*> tracks = player_.queueTracks();

    // If empty, show message
    if (tracks.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        ImGui::TextUnformatted("Queue is empty.");
        ImGui::PopStyleColor();
        return;
    }

    QueueViewActions actions;
    actions.playIndex = [this](std::size_t index) { player_.playQueueIndex(index); };
    actions.moveUp = [this](std::size_t index) { player_.moveQueueTrackUp(index); };
    actions.moveDown = [this](std::size_t index) { player_.moveQueueTrackDown(index); };
    actions.remove = [this](std::size_t index) { player_.removeQueueTrack(index); };

    RenderQueueTable(tracks, player_.currentTrack(), actions);
}