#pragma once
#include <imgui.h>

#include "ui/imgui_widgets.hpp"

enum class ContextView {
    VIEW_QUEUE,
    VIEW_PLAYLIST
};

class TabPanel {
public:
    void drawTabBar();
    void drawTabContent();

private:
    void drawPlaylistContent();
    void drawQueueContent();

    ContextView m_activeTab = ContextView::VIEW_PLAYLIST;
};