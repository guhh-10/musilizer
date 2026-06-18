#include <imgui.h>
#include <imgui_internal.h>
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
    titleStr_ = t->getTitle().empty() ? t->getMusicPath().filename().string() : t->getTitle();
    artistStr_.clear();
    for (std::size_t i = 0; i < t->getArtists().size(); ++i) {
        if (i) artistStr_ += ", ";
        artistStr_ += t->getArtists()[i];
    }
}

void PlayerPanel::drawPlayerZone() {
    if (!ImGui::IsAnyItemActive()) {
        if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
            if (player_.playbackState() == PlaybackState::Playing) {
                player_.pause();
            } else {
                player_.resume();
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
            player_.next();
        }
        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
            player_.previous();
        }
        
        // ─── ADDED: Up and Down Arrows for Volume Control ───
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
            float newVol = std::min(player_.volume() + 0.05f, 1.0f);
            player_.setVolume(newVol);
        }
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
            float newVol = std::max(player_.volume() - 0.05f, 0.0f);
            player_.setVolume(newVol);
        }
    }

    ImGuiStyle& style = ImGui::GetStyle();

    // 1. Title
    ImGui::PushFont(FontManager::npTitle());
    ImGui::TextUnformatted(titleStr_.c_str());
    ImGui::PopFont();

    // 2. Artist
    ImGui::PushFont(FontManager::npArtist());
    ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
    ImGui::TextUnformatted(artistStr_.empty() ? "No artist" : artistStr_.c_str());
    ImGui::PopStyleColor();
    ImGui::PopFont();

    ImGui::Dummy(ImVec2(0.0f, 4.0f));

    // 3. Seek bar — real position / duration
    const Track* currentTrack = player_.currentTrack();
    float pos = 0.0f;
    float dur = 1.0f;  // Default to 1.0f to avoid division issues

    if (currentTrack) {
        pos = player_.position();
        dur = static_cast<float>(player_.currentDuration());
        if (dur < 1.0f) dur = 1.0f;
    } else {
        // No track playing - reset position and duration
        pos = 0.0f;
        dur = 1.0f;
        // Reset drag state if we were dragging
        static bool isDraggingSeekBar = false;
        if (isDraggingSeekBar) {
            isDraggingSeekBar = false;
        }
    }

    // Use static tracking for custom widget interaction state
    static bool isDraggingSeekBar = false;
    static float dragPos = 0.0f;

    // If not currently dragging, sync the slider position with the track position
    if (!isDraggingSeekBar) {
        dragPos = pos;
    }

    // If no track, ensure dragPos is 0
    if (!currentTrack) {
        dragPos = 0.0f;
    }

    // Format strings using dragPos
    char elapsedBuf[16], totalBuf[16];
    snprintf(elapsedBuf, sizeof(elapsedBuf), "%02d:%02d", 
            (int)dragPos / 60, (int)dragPos % 60); 
    snprintf(totalBuf,   sizeof(totalBuf),   "%02d:%02d",
            (int)dur / 60, (int)dur % 60);

    ImGui::PushFont(FontManager::npArtist());
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (!window->SkipItems) {
            const float spacingW = 10.0f;
            
            // Calculate text sizes using the MAX possible width (mm:ss format)
            const char* maxTimeStr = "88:88";
            ImVec2 maxTimeSz = ImGui::CalcTextSize(maxTimeStr, nullptr, true);
            
            float usableW = ImGui::GetWindowContentRegionMax().x
                            - ImGui::GetWindowContentRegionMin().x;
            float sliderW = usableW - maxTimeSz.x * 2.0f - spacingW * 2.0f;
            if (sliderW < 10.0f) sliderW = 10.0f;

            const float entryH = 20.0f;
            ImVec2 basePos = window->DC.CursorPos;
            ImRect bb(basePos, ImVec2(basePos.x + usableW, basePos.y + entryH));
            ImGui::ItemSize(bb, style.FramePadding.y);

            if (ImGui::ItemAdd(bb, window->GetID("##seek_bar"))) {
                float centerY = (bb.Min.y + bb.Max.y) * 0.5f;
                ImU32 mutedCol = ImGui::GetColorU32(style.Colors[ImGuiCol_TextDisabled]);

                // Elapsed
                ImVec2 elapsedSz = ImGui::CalcTextSize(elapsedBuf, nullptr, true);
                window->DrawList->AddText(
                    ImVec2(bb.Min.x, centerY - elapsedSz.y * 0.5f), mutedCol, elapsedBuf);

                // Only draw slider if there's a track, otherwise draw a disabled-looking bar
                if (currentTrack) {
                    // Slider
                    float sliderStartX = bb.Min.x + maxTimeSz.x + spacingW;
                    window->DC.CursorPos = ImVec2(sliderStartX, bb.Min.y);

                    if (SmoothSliderBare("##seek_bar_slider", &dragPos, 0.0f, dur, sliderW)) {
                        isDraggingSeekBar = true;
                    }

                    // If we were dragging, but the user let go of the mouse button
                    if (isDraggingSeekBar && !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                        player_.seek(dragPos);
                        isDraggingSeekBar = false;
                    }

                    // Total
                    float sliderEndX = sliderStartX + sliderW;
                    ImVec2 totalSz = ImGui::CalcTextSize(totalBuf, nullptr, true);
                    window->DrawList->AddText(
                        ImVec2(sliderEndX + spacingW, centerY - totalSz.y * 0.5f),
                        mutedCol, totalBuf);
                } else {
                    // No track - draw a disabled gray bar
                    float sliderStartX = bb.Min.x + maxTimeSz.x + spacingW;
                    float sliderEndX = sliderStartX + sliderW;
                    ImU32 bgColor = ImGui::GetColorU32(style.Colors[ImGuiCol_FrameBg]);
                    ImU32 disabledColor = ImGui::GetColorU32(style.Colors[ImGuiCol_TextDisabled]);
                    disabledColor = IM_COL32(
                        (disabledColor >> 0) & 0xFF,
                        (disabledColor >> 8) & 0xFF,
                        (disabledColor >> 16) & 0xFF,
                        60  // Reduced alpha for disabled look
                    );
                    
                    // Draw a simple progress bar with no interaction
                    window->DrawList->AddRectFilled(
                        ImVec2(sliderStartX, centerY - 2.0f),
                        ImVec2(sliderEndX, centerY + 2.0f),
                        bgColor, 99.0f);
                    
                    // Draw total time
                    ImVec2 totalSz = ImGui::CalcTextSize(totalBuf, nullptr, true);
                    window->DrawList->AddText(
                        ImVec2(sliderEndX + spacingW, centerY - totalSz.y * 0.5f),
                        mutedCol, totalBuf);
                }

                window->DC.CursorPos = ImVec2(basePos.x, bb.Max.y + style.ItemSpacing.y);
            }
        }
    }
    ImGui::PopFont();

    // 4. Media controls — wired to Player
    ImGui::Dummy(ImVec2(0.0f, 4.0f));

    const bool shuffle  = player_.isShuffle();
    const bool repeat   = player_.isRepeat();
    const bool playing  = player_.playbackState() == PlaybackState::Playing;

    const float spacing       = style.ItemSpacing.x + 4.0f;
    const float normalBtnSize = 28.0f;
    const float playBtnSize   = 36.0f;
    const float controlsWidth = (normalBtnSize * 4.0f) + playBtnSize + (spacing * 4.0f);
    const float usableW       = ImGui::GetWindowContentRegionMax().x
                                - ImGui::GetWindowContentRegionMin().x;

    float startX = ImGui::GetCursorPosX() + (usableW - controlsWidth) * 0.5f;
    float startY = ImGui::GetCursorPosY();
    const float normalYOff = (playBtnSize - normalBtnSize) * 0.5f;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

    // Shuffle
    ImGui::SetCursorPos(ImVec2(startX, startY + normalYOff));
    ImGui::PushFont(FontManager::icons());
    if (SmoothRadioButton(ICON_LC_SHUFFLE, ImVec2(normalBtnSize, normalBtnSize),
                          ButtonFont::Icons, shuffle))
        player_.setShuffle(!shuffle);
    ImGui::PopFont();
    startX += normalBtnSize + spacing;

    // Previous
    ImGui::SetCursorPos(ImVec2(startX, startY + normalYOff));
    ImGui::PushFont(FontManager::icons());
    if (SmoothScaleButton(ICON_LC_SKIP_BACK, ImVec2(normalBtnSize, normalBtnSize),
                          ButtonFont::Icons))
        player_.previous();
    ImGui::PopFont();
    startX += normalBtnSize + spacing;

    // Play / Pause
    ImGui::SetCursorPos(ImVec2(startX, startY));
    ImGui::PushFont(FontManager::largeIcons());
    if (SmoothScaleButton(playing ? ICON_LC_PAUSE : ICON_LC_PLAY,
                          ImVec2(playBtnSize, playBtnSize), ButtonFont::LargeIcons)) {
        if (playing)
            player_.pause();
        else
            player_.resume();
    }
    ImGui::PopFont();
    startX += playBtnSize + spacing;

    // Next
    ImGui::SetCursorPos(ImVec2(startX, startY + normalYOff));
    ImGui::PushFont(FontManager::icons());
    if (SmoothScaleButton(ICON_LC_SKIP_FORWARD, ImVec2(normalBtnSize, normalBtnSize),
                          ButtonFont::Icons))
        player_.next();
    ImGui::PopFont();
    startX += normalBtnSize + spacing;

    // Repeat
    ImGui::SetCursorPos(ImVec2(startX, startY + normalYOff));
    ImGui::PushFont(FontManager::icons());
    if (SmoothRadioButton(ICON_LC_REPEAT, ImVec2(normalBtnSize, normalBtnSize),
                          ButtonFont::Icons, repeat))
        player_.setRepeat(!repeat);
    ImGui::PopFont();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    ImGui::SetCursorPosY(startY + playBtnSize + style.ItemSpacing.y);

    // 5. Volume slider — real volume
    float vol = player_.volume();

    const char* volIcon = ICON_LC_VOLUME_OFF;
    if (vol > 0.5f)      volIcon = ICON_LC_VOLUME_2;
    else if (vol > 0.0f) volIcon = ICON_LC_VOLUME_1;

    ImGui::Dummy(ImVec2(0.0f, 4.0f));

    ImGui::PushFont(FontManager::icons());
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (!window->SkipItems) {
            const float spacingW = 10.0f;
            ImVec2 iconSz = ImGui::CalcTextSize(volIcon, nullptr, true);
            float  volUsableW = ImGui::GetWindowContentRegionMax().x
                                - ImGui::GetWindowContentRegionMin().x;
            float  sliderW = volUsableW - iconSz.x - spacingW - 6.0f;
            if (sliderW < 10.0f) sliderW = 10.0f;

            const float entryH = 20.0f;
            ImVec2 basePos = window->DC.CursorPos;
            ImRect bb(basePos, ImVec2(basePos.x + volUsableW, basePos.y + entryH));
            ImGui::ItemSize(bb, style.FramePadding.y);

            if (ImGui::ItemAdd(bb, window->GetID("##vol_zone"))) {
                float centerY  = (bb.Min.y + bb.Max.y) * 0.5f;
                ImU32 mutedCol = ImGui::GetColorU32(style.Colors[ImGuiCol_TextDisabled]);
                window->DrawList->AddText(
                    ImVec2(bb.Min.x, centerY - iconSz.y * 0.5f), mutedCol, volIcon);

                float sliderStartX = bb.Min.x + iconSz.x + spacingW;
                window->DC.CursorPos = ImVec2(sliderStartX, bb.Min.y);

                if (SmoothSliderBare("##vol_slider", &vol, 0.0f, 1.0f, sliderW))
                    player_.setVolume(vol);

                window->DC.CursorPos = ImVec2(basePos.x, bb.Max.y + style.ItemSpacing.y);
            }
        }
    }
    ImGui::PopFont();
}

void PlayerPanel::drawAlbumArt() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    float usableW = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
    float usableH = ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y;
    float artSize = std::min(usableW, usableH);
    if (artSize < 10.0f) artSize = 10.0f;

    float startX = ImGui::GetCursorPosX() + (usableW - artSize) * 0.5f;
    ImGui::SetCursorPosX(startX);

    ImVec2 artTL = ImGui::GetCursorScreenPos();
    ImVec2 artBR = ImVec2(artTL.x + artSize, artTL.y + artSize);

    ImGui::InvisibleButton("##album_art_slot", ImVec2(artSize, artSize));

    ImU32 bgCol   = ImGui::GetColorU32(ImGuiCol_FrameBg);
    ImU32 iconCol = ImGui::GetColorU32(ImGuiCol_TextDisabled);
    drawList->AddRectFilled(artTL, artBR, bgCol, 8.0f);

    const char* icon    = ICON_LC_MUSIC_4;
    ImFont*     iFont   = FontManager::largeIcons();
    float       fontSize = iFont->FontSize;
    ImVec2      iSz     = iFont->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, icon);
    ImVec2      iPos    = ImVec2(
        artTL.x + (artSize - iSz.x) * 0.5f,
        artTL.y + (artSize - iSz.y) * 0.5f);
    drawList->AddText(iFont, fontSize, iPos, iconCol, icon);
}