#pragma once

#include <string>
#include <vector>

#include "controller/player.hpp"
#include "controller/search_controller.hpp"
#include "service/search.hpp"

// Draws the main track browser on the right side:
// search bar at top, scrollable track list below.
// Double-click a track to play it; right-click for a context menu
// (Play, Queue Next, Queue Last, Add to Playlist).

// Sort cycle per column: NEUTRAL -> ASC -> DESC -> NEUTRAL
enum class SortCycleState {
    NEUTRAL, // original insertion / relevance order
    ASC,
    DESC
};

class LibraryPanel {
    public:
        LibraryPanel(Player& player, SearchController& search);

        void drawSearchBar(float padding);
        void drawTableTrack();

        // Set an artist filter (empty string clears it).
        // Triggers an immediate requery.
        void setArtistFilter(const std::string& artist);

    private:
        Player&           player_;
        SearchController& search_;

        char                      searchBuf_[256] = {};
        std::string               artistFilter_;
        std::vector<SearchResult> results_;

        // --- Sort state ---
        // Which column is actively sorted (-1 = none / neutral)
        int             sortCol_   = -1;
        SortCycleState  sortState_ = SortCycleState::NEUTRAL;

        void runSearch();
        void applySortToQuery(SearchQuery& q) const;
        void cycleSort(int col);          // advances the cycle for a given column
        void drawContextMenu(const Track* track);
        void drawColumnHeader(int colIdx, const char* label, float rightPad = 0.0f, float indent = 0.0f);
};
