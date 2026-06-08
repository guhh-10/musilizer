#pragma once

#include <string>

#include "controller/player.hpp"

// Draws the playlist sidebar on the right:
// list of playlists, expand to see tracks, play / delete buttons,
// new playlist input.

class PlaylistPanel {
    public:
        explicit PlaylistPanel(Player& player);

        void draw();

    private:
        Player& player_;

        char newPlaylistName_[128] = {};
        int  selectedPlaylist_     = -1;
};