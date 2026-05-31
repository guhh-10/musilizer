#pragma once
#include <nlohmann/json.hpp>

#include "model/playlist.hpp"
#include "model/play_history.hpp"

namespace Persistence {
    void init();
    void saveSettings(float volume, bool shuffle, bool repeat);
    void loadSettings(float& volume, bool& shuffle, bool& repeat);
    void savePlaylists(const std::vector<Playlist>& playlists);
    std::vector<Playlist> loadPlaylists(const Library& lib);
    void saveHistory(const PlayHistory& history);
    void loadHistory(PlayHistory& history, const Library& lib);
}