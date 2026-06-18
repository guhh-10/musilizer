#pragma once
#include <functional>
#include <string>
#include <imgui.h>

#include "controller/player.hpp"
#include "ui/imgui_widgets.hpp"

enum class ContextView {
    VIEW_QUEUE,
    VIEW_PLAYLIST
};

class TabPanel {
public:
    explicit TabPanel(Player& player);

    void drawTabBar();
    void drawTabContent();

    // Called when the user creates a playlist via the new-playlist input.
    // Wired by MainWindow to player_.addPlaylist().
    // (Already handled internally; kept for future override use.)

private:
    void drawPlaylistContent();
    void drawQueueContent();

    Player& player_;
    ContextView m_activeTab = ContextView::VIEW_PLAYLIST;

};