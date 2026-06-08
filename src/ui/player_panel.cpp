#include <imgui.h>
#include <string>
#include <cstdio>
#include <algorithm>

#include "ui/player_panel.hpp"

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
    const float panelH = ImGui::GetContentRegionAvail().y;
    ImGui::SetCursorPosY((panelH - ImGui::GetTextLineHeightWithSpacing() * 2) * 0.5f);

    // ── Track info ────────────────────────────────────────────────────────────

    ImGui::SetCursorPosX(12.0f);
    ImGui::BeginGroup();
    ImGui::TextUnformatted(titleStr_.c_str());
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
    ImGui::TextUnformatted(artistStr_.c_str());
    ImGui::PopStyleColor();
    ImGui::EndGroup();

    // ── Transport controls ────────────────────────────────────────────────────

    const float centerX = ImGui::GetContentRegionAvail().x * 0.5f;
    ImGui::SameLine(centerX - 80.0f);

    if (ImGui::Button("  |<  ")) player_.previous();
    ImGui::SameLine();

    const bool playing = player_.playbackState() == PlaybackState::Playing;
    if (ImGui::Button(playing ? "  ||  " : "  >  ")) {
        if (playing) player_.pause();
        else         player_.resume();
    }
    ImGui::SameLine();

    if (ImGui::Button("  >|  ")) player_.next();

    // ── Shuffle / Repeat ──────────────────────────────────────────────────────

    ImGui::SameLine(0, 16.0f);

    {
        bool shuffle = player_.isShuffle();
        if (shuffle) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 0.4f, 1.0f));
        if (ImGui::Button("Shuffle")) player_.setShuffle(!shuffle);
        if (shuffle) ImGui::PopStyleColor();
    }

    ImGui::SameLine();

    {
        bool repeat = player_.isRepeat();
        if (repeat) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 0.4f, 1.0f));
        if (ImGui::Button("Repeat")) player_.setRepeat(!repeat);
        if (repeat) ImGui::PopStyleColor();
    }

    // ── Seek slider ───────────────────────────────────────────────────────────

    ImGui::SameLine(0, 16.0f);

    {
        // 1. Determine if the progress bar should be locked
        const Track* currentTrack = player_.currentTrack();
        bool isDisabled = (currentTrack == nullptr);

        // 2. Wrap the widget inside ImGui's disabling scope
        if (isDisabled) {
            ImGui::BeginDisabled(true);
        }

        float maxDuration = currentTrack
            ? static_cast<float>(currentTrack->getDuration())
            : 0.0f;
            
        float currentPos = player_.position();

        char timeBuf[64] = "0:00 / 0:00";
        if (maxDuration > 0.0f) {
            int curMin = static_cast<int>(currentPos) / 60;
            int curSec = static_cast<int>(currentPos) % 60;
            int maxMin = static_cast<int>(maxDuration) / 60;
            int maxSec = static_cast<int>(maxDuration) % 60;
            std::snprintf(timeBuf, sizeof(timeBuf), "%d:%02d / %d:%02d", curMin, curSec, maxMin, maxSec);
        }

        ImGui::SetNextItemWidth(200.0f);
        if (ImGui::SliderFloat("##seek", &currentPos, 0.0f, maxDuration > 0.0f ? maxDuration : 1.0f, timeBuf)) {
            player_.seek(currentPos);
        }

        // 3. Close the disabling scope
        if (isDisabled) {
            ImGui::EndDisabled();
        }
    }

    // ── Volume slider ─────────────────────────────────────────────────────────

    ImGui::SameLine(0, 16.0f);
    ImGui::TextUnformatted("Vol");
    ImGui::SameLine();

    float vol = player_.volume();
    ImGui::SetNextItemWidth(80.0f);
    if (ImGui::SliderFloat("##vol", &vol, 0.0f, 1.0f, "")) {
        player_.setVolume(vol);
    }
}