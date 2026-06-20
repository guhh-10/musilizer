#pragma once

#include "controller/player.hpp"
#include "controller/search_controller.hpp"
#include "ui/player_panel.hpp"
#include "ui/library_panel.hpp"
#include "ui/tab_panel.hpp"

// MainWindow owns all panels and lays them out. Call draw() once per frame.

class MainWindow {
    public:
        MainWindow(Player& player, SearchController& search);

        void draw();

    private:
        Player&           player_;
        SearchController& search_;

        PlayerPanel   playerPanel_;
        LibraryPanel  libraryPanel_;
        TabPanel      tabPanel_;
};