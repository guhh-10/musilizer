#include <imgui.h>
#include <IconsLucide.h>
#include <string>
#include <cstdio>
#include <algorithm>

#include "ui/player_panel.hpp"
#include "ui/fonts.hpp"

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

void PlayerPanel::draw() {
    ImGuiStyle& style = ImGui::GetStyle();

    const float padding = style.WindowPadding.x;
    const ImVec2 fullSize = ImGui::GetContentRegionAvail();
    const float panelW = fullSize.x - padding * 2.0f;
    const float panelH = fullSize.y - padding * 2.0f;
    const ImVec2 origin = ImVec2(ImGui::GetCursorPos().x + padding,
                                  ImGui::GetCursorPos().y + padding);

    // ── Keyboard shortcuts (no visual footprint, can run first) ────────────────
    // Do NOT wrap this in (!io.WantCaptureKeyboard).
    // ImGui::Shortcut handles routing and focus prioritization automatically!
    if (ImGui::Shortcut(ImGuiKey_Space, ImGuiInputFlags_RouteGlobal)) {
        if (player_.playbackState() == PlaybackState::Playing) player_.pause();
        else                                                    player_.resume();
    }
    if (ImGui::Shortcut(ImGuiKey_LeftArrow, ImGuiInputFlags_RouteGlobal))  player_.previous();
    if (ImGui::Shortcut(ImGuiKey_RightArrow, ImGuiInputFlags_RouteGlobal)) player_.next();

    // ── Column layout ────────────────────────────────────────────────────────
    //
    // |  track info (left)  |        seek + controls (middle)        | volume (right) |
    //
    const float leftW  = 240.0f;
    const float rightW = 160.0f;
    const float gap    = 16.0f;

    const float middleX = origin.x + leftW + gap;
    const float middleW = panelW - leftW - rightW - gap * 2.0f;

    // ── Left: track info (vertically centered) ─────────────────────────────────

    {
        const float infoH = ImGui::GetTextLineHeightWithSpacing() * 2.0f;
        const float infoY = origin.y + (panelH - infoH) * 0.5f;

        ImGui::SetCursorPos(ImVec2(origin.x, infoY));
        ImGui::BeginGroup();
        ImGui::TextUnformatted(titleStr_.c_str());
        ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
        ImGui::TextUnformatted(artistStr_.c_str());
        ImGui::PopStyleColor();
        ImGui::EndGroup();
    }

    // ── Middle: seek slider on top, duration + transport controls below ───────

    const Track* currentTrack = player_.currentTrack();
    const bool   noTrack      = (currentTrack == nullptr);
    const bool   playing      = player_.playbackState() == PlaybackState::Playing;

    const float seekRowH     = ImGui::GetFrameHeight();
    const float controlsRowH = 36.0f;
    const float rowSpacing   = style.ItemSpacing.y;
    const float middleH      = seekRowH + rowSpacing + controlsRowH;
    const float middleY      = origin.y + (panelH - middleH) * 0.5f;

    // -- Seek slider row --
    {
        if (noTrack) ImGui::BeginDisabled(true);

        float maxDuration = currentTrack ? static_cast<float>(currentTrack->getDuration()) : 0.0f;
        float currentPos  = player_.position();

        ImGui::SetCursorPos(ImVec2(middleX, middleY));
        ImGui::SetNextItemWidth(middleW);
        if (ImGui::SliderFloat("##seek", &currentPos, 0.0f, maxDuration > 0.0f ? maxDuration : 1.0f, "")) {
            player_.seek(currentPos);
        }

        if (noTrack) ImGui::EndDisabled();
    }

    // -- Duration + transport controls row --
    {
        const float row2Y = middleY + seekRowH + rowSpacing;

        // Duration text, left-aligned within the middle column
        float maxDuration = currentTrack ? static_cast<float>(currentTrack->getDuration()) : 0.0f;
        float currentPos  = player_.position();
        char timeBuf[64] = "0:00 / 0:00";
        if (maxDuration > 0.0f) {
            int curMin = static_cast<int>(currentPos) / 60;
            int curSec = static_cast<int>(currentPos) % 60;
            int maxMin = static_cast<int>(maxDuration) / 60;
            int maxSec = static_cast<int>(maxDuration) % 60;
            std::snprintf(timeBuf, sizeof(timeBuf), "%d:%02d / %d:%02d", curMin, curSec, maxMin, maxSec);
        }

        ImGui::SetCursorPos(ImVec2(middleX, row2Y + (controlsRowH - ImGui::GetTextLineHeight()) * 0.5f));
        ImGui::TextUnformatted(timeBuf);

        // Shuffle / prev / play-pause / next / repeat, centered within the
        // middle column with precise square sizes and vertical alignment.
        const float spacing       = style.ItemSpacing.x;
        const float normalBtnSize = 28.0f;
        const float playBtnSize   = 36.0f;

        const float controlsWidth = normalBtnSize * 4.0f + playBtnSize + spacing * 4.0f;
        const float controlsX     = middleX + (middleW - controlsWidth) * 0.5f;

        const float normalY = row2Y + (playBtnSize - normalBtnSize) * 0.5f;
        const float playY   = row2Y;

        if (noTrack) ImGui::BeginDisabled(true);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));

        float curX = controlsX;

        // Shuffle
        ImGui::SetCursorPos(ImVec2(curX, normalY));
        ImGui::PushFont(FontManager::icons());
        {
            bool shuffle = player_.isShuffle();
            if (shuffle) ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_CheckMark]);
            if (ImGui::Button(ICON_LC_SHUFFLE, ImVec2(normalBtnSize, normalBtnSize))) player_.setShuffle(!shuffle);
            if (shuffle) ImGui::PopStyleColor();
        }
        ImGui::PopFont();
        curX += normalBtnSize + spacing;

        // Previous
        ImGui::SetCursorPos(ImVec2(curX, normalY));
        ImGui::PushFont(FontManager::icons());
        if (ImGui::Button(ICON_LC_SKIP_BACK, ImVec2(normalBtnSize, normalBtnSize))) player_.previous();
        ImGui::PopFont();
        curX += normalBtnSize + spacing;

        // Play/Pause (Larger)
        ImGui::SetCursorPos(ImVec2(curX, playY));
        ImGui::PushFont(FontManager::largeIcons());
        if (ImGui::Button(playing ? ICON_LC_PAUSE : ICON_LC_PLAY, ImVec2(playBtnSize, playBtnSize))) {
            if (playing) player_.pause();
            else         player_.resume();
        }
        ImGui::PopFont();
        curX += playBtnSize + spacing;

        // Next
        ImGui::SetCursorPos(ImVec2(curX, normalY));
        ImGui::PushFont(FontManager::icons());
        if (ImGui::Button(ICON_LC_SKIP_FORWARD, ImVec2(normalBtnSize, normalBtnSize))) player_.next();
        ImGui::PopFont();
        curX += normalBtnSize + spacing;

        // Repeat
        ImGui::SetCursorPos(ImVec2(curX, normalY));
        ImGui::PushFont(FontManager::icons());
        {
            bool repeat = player_.isRepeat();
            if (repeat) ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_CheckMark]);
            if (ImGui::Button(ICON_LC_REPEAT, ImVec2(normalBtnSize, normalBtnSize))) player_.setRepeat(!repeat);
            if (repeat) ImGui::PopStyleColor();
        }
        ImGui::PopFont();

        ImGui::PopStyleVar();
        if (noTrack) ImGui::EndDisabled();
    }

    // ── Right: volume slider (aligned with the seek slider row) ────────────────

    {
        const float volX = origin.x + panelW - rightW;

        ImGui::SetCursorPos(ImVec2(volX, middleY + (seekRowH - ImGui::GetTextLineHeight()) * 0.5f));
        ImGui::TextUnformatted("Vol");
        ImGui::SameLine();

        const float labelW = ImGui::CalcTextSize("Vol").x + style.ItemSpacing.x;
        ImGui::SetCursorPos(ImVec2(volX + labelW, middleY));

        float vol = player_.volume();
        ImGui::SetNextItemWidth(rightW - labelW);
        if (ImGui::SliderFloat("##vol", &vol, 0.0f, 1.0f, "")) {
            player_.setVolume(vol);
        }
    }
}