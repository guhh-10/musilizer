#include <imgui.h>
#include <iostream>
#include <fstream>

#include <IconsLucide.h>
#include "ui/fonts.hpp"

void FontManager::init(const std::string& exeDir) {
    ImGuiIO& io = ImGui::GetIO();

    std::string regularPath = exeDir + "/InterVariable.ttf";
    std::string boldPath    = exeDir + "/Inter-Bold.ttf";
    std::string iconPath    = exeDir + "/lucide.ttf";

    // 1. Core fallback fonts (16px) — s_regular becomes the default font
    s_regular = io.Fonts->AddFontFromFileTTF(regularPath.c_str(), 18.0f);
    s_bold    = io.Fonts->AddFontFromFileTTF(boldPath.c_str(),    18.0f);

    if (!s_regular || !s_bold) {
        std::cerr << "ERROR: Failed to load text fonts from " << exeDir << "\n";
        return;
    }

    // Set Inter Regular as the default ImGui font explicitly
    io.FontDefault = s_regular;

    // 2. Pre-bake Now Playing section sizes
    s_np_title  = io.Fonts->AddFontFromFileTTF(boldPath.c_str(),    17.0f); // .np-title
    s_np_artist = io.Fonts->AddFontFromFileTTF(regularPath.c_str(), 14.0f); // .np-artist

    // 3. Pre-bake Up Next Queue sizes
    s_q_title  = io.Fonts->AddFontFromFileTTF(boldPath.c_str(),    15.0f);  // .q-title
    s_q_artist = io.Fonts->AddFontFromFileTTF(regularPath.c_str(), 13.0f);  // .q-artist

    // 4. Pre-bake Library Track List size (title & artist both 13px Regular)
    s_t_title_artist = io.Fonts->AddFontFromFileTTF(regularPath.c_str(), 13.0f); // .t-title & .t-artist

    if (!s_np_title || !s_np_artist || !s_q_title || !s_q_artist || !s_t_title_artist) {
        std::cerr << "ERROR: Failed to pre-bake one or more role fonts from " << exeDir << "\n";
        return;
    }

    // 5. Load icon fonts
    if (!std::ifstream(iconPath)) {
        std::cerr << "ERROR: Icon font file NOT FOUND at " << iconPath << "\n";
        return;
    }

    // Static storage so the pointer stays valid until atlas build time
    static const ImWchar icon_ranges[] = { ICON_MIN_LC, ICON_MAX_LC, 0 };

    ImFontConfig icon_config_small;
    icon_config_small.GlyphRanges = icon_ranges;
    icon_config_small.PixelSnapH  = true;

    ImFontConfig icon_config_large;
    icon_config_large.GlyphRanges = icon_ranges;
    icon_config_large.PixelSnapH  = true;

    s_icons       = io.Fonts->AddFontFromFileTTF(iconPath.c_str(), 18.0f, &icon_config_small);
    s_large_icons = io.Fonts->AddFontFromFileTTF(iconPath.c_str(), 24.0f, &icon_config_large);

    if (!s_icons || !s_large_icons) {
        std::cerr << "ERROR: Failed to load icon fonts from " << iconPath << "\n";
    }
}