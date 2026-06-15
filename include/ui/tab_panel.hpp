#pragma once
#include <imgui.h>
#include <map>

#include "ui/fonts.hpp"

enum class ButtonFont {
    Regular,
    Bold,
    Icons,
    LargeIcons
};

struct SmoothAnimState {
    static float& GetRef(ImGuiID id) {
        static std::map<ImGuiID, float> localMap;
        return localMap[id];
    }
};

bool SmoothRadioButton(const char* label, const ImVec2& size_arg = ImVec2(0, 0), ButtonFont font = ButtonFont::Regular, bool active = false);

class TabPanel {
public:
    void drawTabBar();
    void drawTabContent();

private:
    int m_activeTab = 0;
};