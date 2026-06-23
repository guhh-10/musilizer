#pragma once

#include "controller/player.hpp"
#include "controller/search_controller.hpp"
#include "ui/player_panel.hpp"
#include "ui/library_panel.hpp"
#include "ui/tab_panel.hpp"
#include "ui/waveform_panel.hpp"

class MainWindow {
    public:
        MainWindow(Player& player, SearchController& search);
        void draw();
        // Called when the file watcher changes the library so the UI re-queries.
        void notifyLibraryChanged();

    private:
        Player&           player_;
        SearchController& search_;

        PlayerPanel   playerPanel_;
        LibraryPanel  libraryPanel_;
        TabPanel      tabPanel_;
        WaveformPanel waveformPanel_;
};