#pragma once
#include <imgui.h>
#include <map>
#include <string>
#include <vector>

#include "ui/fonts.hpp"

// ── Shared animation state ──────────────────────────────────────────────────

struct SmoothAnimState {
    static float& GetRef(ImGuiID id) {
        static std::map<ImGuiID, float> localMap;
        return localMap[id];
    }
};

// ── Font selector for custom button widgets ─────────────────────────────────

enum class ButtonFont {
    Regular,
    Bold,
    Icons,
    LargeIcons
};

ImFont* resolveFont(ButtonFont font);

// ── Custom widget declarations ──────────────────────────────────────────────

bool SmoothRadioButton(const char* label, const ImVec2& size_arg = ImVec2(0, 0), ButtonFont font = ButtonFont::Regular, bool active = false);
bool SmoothSliderBare(const char* str_id, float* v, float v_min, float v_max, float slider_bar_width);
bool SmoothScaleButton(const char* label, const ImVec2& size_arg, ButtonFont font = ButtonFont::Icons);
bool SmoothActiveInputText(const char* label, char* buf, size_t buf_size, const ImVec2& size_arg);

// ── SmoothHoverTable ────────────────────────────────────────────────────────

struct TableRowItem {
    std::string number;
    std::string title;
    std::string duration;
};

bool SmoothHoverTable(const char* str_id, const std::vector<TableRowItem>& items, int* out_selected_index = nullptr);