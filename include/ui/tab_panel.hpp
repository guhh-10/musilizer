#pragma once
#include <imgui.h>

#include "ui/imgui_widgets.hpp"

class TabPanel {
public:
    void drawTabBar();
    void drawTabContent();

private:
    int m_activeTab = 0;
};