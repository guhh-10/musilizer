#pragma once

#include <string>
#include <vector>

#include "controller/player.hpp"
#include "controller/search_controller.hpp"
#include "service/search.hpp"

// Draws the main track browser on the left side:
// search bar at top, scrollable track list below.
// Double-click a track to play it; right-click for a context menu
// (Play, Queue Next, Queue Last, Add to Playlist).

class LibraryPanel {
    public:
        LibraryPanel(Player& player, SearchController& search);

        void draw();

    private:
        Player&           player_;
        SearchController& search_;

        char                      searchBuf_[256] = {};
        std::vector<SearchResult> results_;

        void runSearch();
        void drawContextMenu(const Track& track);
};