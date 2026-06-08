#include <imgui.h>

#include "ui/library_panel.hpp"

LibraryPanel::LibraryPanel(Player& player, SearchController& search)
    : player_(player)
    , search_(search)
{
    // Initial population — show everything on startup.
    runSearch();

    // Rebuild search index and refresh results when the library changes.
    // (Currently the library is loaded once at startup, but this hook is
    // ready for a future "rescan" feature.)
}

void LibraryPanel::runSearch() {
    results_ = search_.query(searchBuf_);
}

void LibraryPanel::draw() {
    // ── Search bar ────────────────────────────────────────────────────────────

    ImGui::SetNextItemWidth(-1.0f);
    if (ImGui::InputTextWithHint("##search", "Search tracks, artists...",
                                  searchBuf_, sizeof(searchBuf_)))
    {
        runSearch();
    }

    ImGui::Spacing();

    // ── Track list Table ──────────────────────────────────────────────────────

    // Flags to configure our modern table layout:
    // - ScrollY: Enables internal vertical scrolling (replaces BeginChild)
    // - RowBg: Gives us zebra-striping rows for readability
    // - SizingFixedFit: Lets us mix percentage widths with stretch/fit options
    ImGuiTableFlags tableFlags = ImGuiTableFlags_ScrollY | 
                                 ImGuiTableFlags_RowBg | 
                                 ImGuiTableFlags_PadOuterX;

    // Define a 3-column table that fills all remaining vertical space ({0, 0})
    if (ImGui::BeginTable("##library_table", 3, tableFlags, ImVec2(0, 0))) 
    {
        // 1. Setup Column Widths cleanly using Stretch weights (percentages)
        ImGui::TableSetupColumn("Title", ImGuiTableColumnFlags_WidthStretch, 50.0f);
        ImGui::TableSetupColumn("Artist", ImGuiTableColumnFlags_WidthStretch, 35.0f);
        ImGui::TableSetupColumn("Duration", ImGuiTableColumnFlags_WidthStretch, 15.0f);

        // 2. Render Headers
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        ImGui::TableHeadersRow(); // Automatically prints "Title", "Artist", "Duration"
        ImGui::PopStyleColor();

        // 3. Render Rows
        for (const auto& result : results_) {
            const Track* track = result.track;

            // Move execution into a brand new row
            ImGui::TableNextRow();

            // ── Column 0: Title ───────────────────────────────────────────────
            ImGui::TableSetColumnIndex(0);

            const std::string& title = track->getTitle();
            const std::string  label = (title.empty()
                ? track->getMusicPath().filename().string()
                : title) + "##" + track->getMusicPath().string();

            // SpanAllColumns makes the row highlight cleanly across all 3 columns
            if (ImGui::Selectable(label.c_str(), false,
                                  ImGuiSelectableFlags_SpanAllColumns |
                                  ImGuiSelectableFlags_AllowDoubleClick))
            {
                // Double-click row handler inside your Table rendering iteration loop:
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                    // Collect all current tracks matching the visible search filter list
                    std::vector<const Track*> contextualQueue;
                    for (const auto& res : results_) {
                        contextualQueue.push_back(res.track);
                    }

                    // Load everything into the queue container
                    player_.playPlaylist(Playlist("Contextual Library Queue")); 
                    
                    // Explicitly jump directly to the clicked song index within that setup context
                    player_.play(*track); 
                }
            }

            // Right-click context menu
            drawContextMenu(*track);

            // ── Column 1: Artist ──────────────────────────────────────────────
            ImGui::TableSetColumnIndex(1);

            std::string artistStr;
            for (std::size_t i = 0; i < track->getArtists().size(); ++i) {
                if (i > 0) artistStr += ", ";
                artistStr += track->getArtists()[i];
            }
            ImGui::TextUnformatted(artistStr.c_str());

            // ── Column 2: Duration ────────────────────────────────────────────
            ImGui::TableSetColumnIndex(2);

            int d   = track->getDuration();
            int min = d / 60;
            int sec = d % 60;
            ImGui::Text("%d:%02d", min, sec);
        }

        ImGui::EndTable();
    }
}

void LibraryPanel::drawContextMenu(const Track& track) {
    if (!ImGui::BeginPopupContextItem()) return;

    if (ImGui::MenuItem("Play"))
        player_.play(track);

    if (ImGui::MenuItem("Queue Next"))
        player_.queueNext(track);

    if (ImGui::MenuItem("Queue Last"))
        player_.queueLast(track);

    ImGui::Separator();

    // Add to playlist submenu
    if (ImGui::BeginMenu("Add to Playlist")) {
        if (player_.playlists().empty()) {
            ImGui::TextDisabled("No playlists");
        } else {
            for (const Playlist& pl : player_.playlists()) {
                if (ImGui::MenuItem(pl.getName().c_str()))
                    player_.addTrackToPlaylist(pl.getName(), track);
            }
        }
        ImGui::EndMenu();
    }

    ImGui::EndPopup();
}