#include <imgui.h>
#include <imgui_internal.h>
#include <vector>
#include <string>
#include <cstdio>

#include "ui/tab_panel.hpp"
#include "ui/imgui_widgets.hpp"

// ── helpers ───────────────────────────────────────────────────────────────────

static std::string formatDurationShort(int secs) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d:%02d", secs / 60, secs % 60);
    return buf;
}

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

    for (const Playlist& pl : playlists) {
        PlaylistGroup g;
        g.name = pl.getName();
        for (const fs::path& p : pl.getPlaylistTracks()) {
            PlaylistItemTrack item;
            item.name     = p.stem().string();
            item.duration = "";
            g.tracks.push_back(std::move(item));
        }
        groups.push_back(std::move(g));
    }

    ImGui::BeginChild("##tab_pl_scroll", ImVec2(0.0f, 0.0f), false);

    if (groups.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        ImGui::TextUnformatted("No playlists yet.");
        ImGui::PopStyleColor();
    } else {
        // PASS player_ HERE:
        StrictTwoTierPlaylistView("##pl_tree", groups, player_);
    }

    ImGui::EndChild();
}

// ── Queue tab ─────────────────────────────────────────────────────────────────

void TabPanel::drawQueueContent()
{
    const std::vector<const Track*> tracks = player_.queueTracks();

    std::vector<TableRowItem> rows;
    rows.reserve(tracks.size());

    for (std::size_t i = 0; i < tracks.size(); ++i) {
        const Track* t = tracks[i];
        TableRowItem item;
        item.number = std::to_string(i + 1);
        if (t) {
            item.title    = t->getTitle().empty()
                              ? t->getMusicPath().filename().string()
                              : t->getTitle();
            
            // --- FIXED: Loop through and comma-separate all artists ---
            std::string artistStr;
            for (std::size_t a = 0; a < t->getArtists().size(); ++a) {
                if (a) artistStr += ", ";
                artistStr += t->getArtists()[a];
            }
            item.artist   = artistStr;
            
            item.duration = formatDurationShort(t->getDuration());
        } else {
            item.title    = "(unknown)";
            item.artist   = "";
            item.duration = "";
        }
        rows.push_back(std::move(item));
    }

    if (rows.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        ImGui::TextUnformatted("Queue is empty.");
        ImGui::PopStyleColor();
        return;
    }

    // Track which item is clicked using SmoothHoverTable's return value
    static int selectedQueueIdx = -1;
    if (SmoothHoverTable("##queue_table", rows, &selectedQueueIdx)) {
        // Simple click registered!
        if (selectedQueueIdx >= 0 && selectedQueueIdx < (int)tracks.size()) {
            player_.playQueueIndex(static_cast<std::size_t>(selectedQueueIdx));
        }
    }
}