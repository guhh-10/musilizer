#include <imgui.h>
#include <imgui_internal.h> // Required for ImRect and window->DC access
#include <IconsLucide.h>
#include <string>
#include <cstdio>
#include <algorithm>

#include "ui/player_panel.hpp"
#include "ui/fonts.hpp"
#include "ui/imgui_widgets.hpp"

PlayerPanel::PlayerPanel(Player& player) : player_(player) {
    player_.onTrackChanged = [this](const Track* t) {
        updateTrackStrings(t);
    };

    updateTrackStrings(player_.currentTrack());
}

void PlayerPanel::updateTrackStrings(const Track* t) {
    if (!t) {
        titleStr_  = "No track";
        artistStr_ = "";
        return;
    }
    titleStr_ = t->getTitle();
    if (titleStr_.empty()) titleStr_ = t->getMusicPath().filename().string();

    artistStr_.clear();
    for (std::size_t i = 0; i < t->getArtists().size(); ++i) {
        if (i > 0) artistStr_ += ", ";
        artistStr_ += t->getArtists()[i];
    }
}

void PlayerPanel::drawPlayerZone() {
    ImGuiStyle& style = ImGui::GetStyle();

    // 1. Render Track Title
    ImGui::PushFont(FontManager::npTitle());
    ImGui::TextUnformatted(titleStr_.c_str());
    ImGui::PopFont();

    // 2. Render Artist Metadata
    ImGui::PushFont(FontManager::npArtist());
    ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
    if (!artistStr_.empty()) {
        ImGui::TextUnformatted(artistStr_.c_str());
    } else {
        ImGui::TextUnformatted("No artist");
    }
    ImGui::PopStyleColor();
    ImGui::PopFont();

    ImGui::Dummy(ImVec2(0.0f, 4.0f));

    // 3. Static/Mock Timeline Variables
    static float static_playback_progress = 34.0f; 
    float total_track_duration = 215.0f;          
    
    char elapsed_fmt[16];
    char total_fmt[16];
    sprintf(elapsed_fmt, "%02d:%02d", (int)static_playback_progress / 60, (int)static_playback_progress % 60);
    sprintf(total_fmt, "%02d:%02d", (int)total_track_duration / 60, (int)total_track_duration % 60);

    // 4. Inline Seek Bar Implementation
    ImGui::PushFont(FontManager::npArtist());
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (!window->SkipItems) 
        {
            // CONVENTION FIX: Removed 'ImGuiContext& g' to satisfy -Werror=unused-variable
            float spacing_width = 10.0f; 

            ImVec2 left_text_size  = ImGui::CalcTextSize(elapsed_fmt, NULL, true);
            ImVec2 right_text_size = ImGui::CalcTextSize(total_fmt, NULL, true);
            
            // Matches your dynamic stretch calculations from tab_panel.cpp
            float total_usable_width = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;

            float slider_bar_width = total_usable_width - (left_text_size.x + spacing_width + spacing_width + right_text_size.x);
            if (slider_bar_width < 10.0f) slider_bar_width = 10.0f;

            float entry_height = 20.0f; 
            ImVec2 base_pos = window->DC.CursorPos;

            ImRect bb(base_pos, ImVec2(base_pos.x + total_usable_width, base_pos.y + entry_height));
            ImGui::ItemSize(bb, style.FramePadding.y);
            
            if (ImGui::ItemAdd(bb, window->GetID("##track_timeline_static"))) 
            {
                float centerY = (bb.Min.y + bb.Max.y) * 0.5f;
                ImU32 muted_color = ImGui::GetColorU32(style.Colors[ImGuiCol_TextDisabled]);

                // Draw Left Timestamp
                window->DrawList->AddText(ImVec2(bb.Min.x, centerY - (left_text_size.y * 0.5f)), muted_color, elapsed_fmt);

                // Slider drawing
                float slider_start_x = bb.Min.x + left_text_size.x + spacing_width;
                window->DC.CursorPos = ImVec2(slider_start_x, bb.Min.y);

                SmoothSliderBare("##track_timeline_static", &static_playback_progress, 0.0f, total_track_duration, slider_bar_width);

                // Draw Right Timestamp
                float slider_end_x = slider_start_x + slider_bar_width;
                window->DrawList->AddText(ImVec2(slider_end_x + spacing_width, centerY - (right_text_size.y * 0.5f)), muted_color, total_fmt);

                // Safe layout cursor restore point
                window->DC.CursorPos = ImVec2(base_pos.x, bb.Max.y + style.ItemSpacing.y);
            }
        }
    }
    ImGui::PopFont();

    // 5. Media Controllers
    ImGui::Dummy(ImVec2(0.0f, 4.0f));

    static bool static_shuffle = false;
    static bool static_repeat  = false;
    static bool static_playing = false;

    const float spacing       = style.ItemSpacing.x + 4.0f;
    const float normalBtnSize = 28.0f;
    const float playBtnSize   = 36.0f;

    // Calculate total layout width to center the controller row
    const float controlsWidth = (normalBtnSize * 4.0f) + playBtnSize + (spacing * 4.0f);
    const float total_usable_width = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;

    float startX = ImGui::GetCursorPosX() + (total_usable_width - controlsWidth) * 0.5f;
    float startY = ImGui::GetCursorPosY();

    const float normalYOffset = (playBtnSize - normalBtnSize) * 0.5f;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
    // Encase all buttons with a white text color
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 1.00f, 1.00f, 1.00f));

    // Shuffle Button — SmoothRadioButton (toggles active state)
    ImGui::SetCursorPos(ImVec2(startX, startY + normalYOffset));
    ImGui::PushFont(FontManager::icons());
    if (SmoothRadioButton(ICON_LC_SHUFFLE, ImVec2(normalBtnSize, normalBtnSize), ButtonFont::Icons, static_shuffle))
        static_shuffle = !static_shuffle;
    ImGui::PopFont();
    startX += normalBtnSize + spacing;

    // Previous Button — SmoothScaleButton
    ImGui::SetCursorPos(ImVec2(startX, startY + normalYOffset));
    ImGui::PushFont(FontManager::icons());
    // FIXED: Added ButtonFont::Icons as the 3rd argument
    SmoothScaleButton(ICON_LC_SKIP_BACK, ImVec2(normalBtnSize, normalBtnSize), ButtonFont::Icons); 
    ImGui::PopFont();
    startX += normalBtnSize + spacing;

    // Play / Pause Button — SmoothScaleButton (larger)
    ImGui::SetCursorPos(ImVec2(startX, startY));
    ImGui::PushFont(FontManager::largeIcons());
    // FIXED: Added ButtonFont::LargeIcons as the 3rd argument
    if (SmoothScaleButton(static_playing ? ICON_LC_PAUSE : ICON_LC_PLAY, ImVec2(playBtnSize, playBtnSize), ButtonFont::LargeIcons))
        static_playing = !static_playing;
    ImGui::PopFont();
    startX += playBtnSize + spacing;

    // Next Button — SmoothScaleButton
    ImGui::SetCursorPos(ImVec2(startX, startY + normalYOffset));
    ImGui::PushFont(FontManager::icons());
    // FIXED: Added ButtonFont::Icons as the 3rd argument
    SmoothScaleButton(ICON_LC_SKIP_FORWARD, ImVec2(normalBtnSize, normalBtnSize), ButtonFont::Icons);
    ImGui::PopFont();
    startX += normalBtnSize + spacing;

    // Repeat Button — SmoothRadioButton (toggles active state)
    ImGui::SetCursorPos(ImVec2(startX, startY + normalYOffset));
    ImGui::PushFont(FontManager::icons());
    if (SmoothRadioButton(ICON_LC_REPEAT, ImVec2(normalBtnSize, normalBtnSize), ButtonFont::Icons, static_repeat))
        static_repeat = !static_repeat;
    ImGui::PopFont();

    // Pop the style color and var in reverse order of pushing
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    // Advance cursor past the button row height
    ImGui::SetCursorPosY(startY + playBtnSize + style.ItemSpacing.y);

    // 6. Volume Control Implementation
    static float static_volume = 0.5f; // Initial volume state (0.0f to 1.0f)

    // Select icon state dynamically depending on current volume level
    const char* volume_icon = ICON_LC_VOLUME_OFF;
    if (static_volume > 0.5f) {
        volume_icon = ICON_LC_VOLUME_2;
    } else if (static_volume > 0.0f) {
        volume_icon = ICON_LC_VOLUME_1;
    }

    ImGui::Dummy(ImVec2(0.0f, 4.0f));

    ImGui::PushFont(FontManager::icons());
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (!window->SkipItems) 
        {
            float spacing_width = 10.0f; 
            ImVec2 icon_size = ImGui::CalcTextSize(volume_icon, NULL, true);
            
            // Isolate usable region calculation to prevent duplicate token errors from section 5
            float volume_usable_width = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;

            float right_padding = 6.0f;

            float slider_bar_width = volume_usable_width - (icon_size.x + spacing_width + right_padding);
            if (slider_bar_width < 10.0f) slider_bar_width = 10.0f;

            float entry_height = 20.0f; 
            ImVec2 base_pos = window->DC.CursorPos;

            ImRect bb(base_pos, ImVec2(base_pos.x + volume_usable_width, base_pos.y + entry_height));
            ImGui::ItemSize(bb, style.FramePadding.y);
            
            if (ImGui::ItemAdd(bb, window->GetID("##volume_slider_zone"))) 
            {
                float centerY = (bb.Min.y + bb.Max.y) * 0.5f;
                ImU32 muted_color = ImGui::GetColorU32(style.Colors[ImGuiCol_TextDisabled]);

                // Render Volume Icon on the left
                window->DrawList->AddText(ImVec2(bb.Min.x, centerY - (icon_size.y * 0.5f)), muted_color, volume_icon);

                // Align structural cursor position to the start of the slider track
                float slider_start_x = bb.Min.x + icon_size.x + spacing_width;
                window->DC.CursorPos = ImVec2(slider_start_x, bb.Min.y);

                // Invoke your custom Smooth Slider
                SmoothSliderBare("##volume_slider_bar", &static_volume, 0.0f, 1.0f, slider_bar_width);

                // Safe layout cursor restore point to respect trailing layouts
                window->DC.CursorPos = ImVec2(base_pos.x, bb.Max.y + style.ItemSpacing.y);
            }
        }
    }
    ImGui::PopFont();
}

void PlayerPanel::drawAlbumArt() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
 
    // 1. Calculate usable dimensions and target square art size
    float total_usable_width  = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
    float total_usable_height = ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y;
    float artSize = std::min(total_usable_width, total_usable_height);
    if (artSize < 10.0f) artSize = 10.0f;

    // 2. Center the layout horizontally within the available panel width
    float leftover_x = total_usable_width - artSize;
    float startX = ImGui::GetCursorPosX() + (leftover_x * 0.5f);
    
    // Set cursor position to the centered starting point
    ImGui::SetCursorPosX(startX);

    // 3. Capture screen-space coordinates for the DrawList operations
    ImVec2 artTopLeft = ImGui::GetCursorScreenPos();
    ImVec2 artBottomRight = ImVec2(artTopLeft.x + artSize, artTopLeft.y + artSize);

    // 4. Reserve bounding box layout space using the exact centered dimensions
    ImGui::InvisibleButton("##album_art_slot", ImVec2(artSize, artSize));
 
    // 5. Render Background Card
    ImU32 bg_color = ImGui::GetColorU32(ImGuiCol_FrameBg);
    ImU32 icon_color = ImGui::GetColorU32(ImGuiCol_TextDisabled);
    drawList->AddRectFilled(artTopLeft, artBottomRight, bg_color, 8.0f);
 
    // 6. Center and Render Placeholder Icon
    const char* placeholder_icon = ICON_LC_MUSIC_4;
    ImFont* iconFont = FontManager::largeIcons();
    float fontSize = iconFont->FontSize; 
 
    ImVec2 icon_text_size = iconFont->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, placeholder_icon);
 
    ImVec2 icon_pos = ImVec2(
        artTopLeft.x + (artSize - icon_text_size.x) * 0.5f,
        artTopLeft.y + (artSize - icon_text_size.y) * 0.5f
    );
 
    drawList->AddText(iconFont, fontSize, icon_pos, icon_color, placeholder_icon);
}