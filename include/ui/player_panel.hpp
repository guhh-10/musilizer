#pragma once

#include "controller/player.hpp"

// Draws the now-playing bar at the bottom of the window:
// album art placeholder | title + artist | prev / play-pause / next |
// shuffle | repeat | seek slider | volume slider

class PlayerPanel {
    public:
        explicit PlayerPanel(Player& player);

        void draw();

    private:
        Player& player_;

        // Cached display strings — rebuilt in onTrackChanged so we don't
        // call c_str() on temporaries every frame.
        std::string titleStr_;
        std::string artistStr_;

        void updateTrackStrings(const Track* t);
};