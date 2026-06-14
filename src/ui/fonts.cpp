#include <imgui.h>
#include <iostream>
#include <fstream>

#include <IconsLucide.h>
#include "ui/fonts.hpp"

void FontManager::init(const std::string& exeDir) {
    ImGuiIO& io = ImGui::GetIO();

    // Regular font
    std::string regularPath = exeDir + "/InterVariable.ttf";
    s_regular = io.Fonts->AddFontFromFileTTF(regularPath.c_str(), 16.0f);

    // Bold font
    std::string boldPath = exeDir + "/Inter-Bold.ttf";
    s_bold = io.Fonts->AddFontFromFileTTF(boldPath.c_str(), 16.0f);

    // Icon font (standalone, NOT merged into regular)
    std::string iconPath = exeDir + "/lucide.ttf";

    if (!std::ifstream(iconPath)) {
        std::cerr << "Icon font file NOT FOUND at " << iconPath << "\n";
    }

    // IMPORTANT: tell ImGui which codepoints to actually bake into the atlas.
    // ICON_MIN_LC / ICON_MAX_LC come from IconsLucide.h
    static const ImWchar icon_ranges[] = { ICON_MIN_LC, ICON_MAX_LC, 0 };

    ImFontConfig icon_config_small;
    icon_config_small.GlyphRanges = icon_ranges;
    icon_config_small.PixelSnapH = true;
    // icon_config_small.GlyphOffset.y = 2.0f; // optional vertical nudge if icons look misaligned with text

    ImFontConfig icon_config_large;
    icon_config_large.GlyphRanges = icon_ranges;
    icon_config_large.PixelSnapH = true;

    s_icons = io.Fonts->AddFontFromFileTTF(iconPath.c_str(), 16.0f, &icon_config_small);
    s_large_icons = io.Fonts->AddFontFromFileTTF(iconPath.c_str(), 22.0f, &icon_config_large);
    if (!s_icons || !s_large_icons) {
        std::cerr << "ERROR: Failed to load icon fonts from " << iconPath << "\n";
    } else {
        std::cout << "Icon fonts loaded successfully\n";
    }
}