#pragma once
#include <nlohmann/json.hpp>

#include "model/playlist.hpp"
#include "model/playHistory.hpp"

class persistence{
    public:
        static void init();
        static void saveSettings(float volume, bool shuffle, bool repeat);
        static void loadSettings(float& volume, bool& shuffle, bool& repeat);
        static void savePlaylists(const std::vector<playlist>& playlists);
        static std::vector<playlist> loadPlaylists(const library& lib);
        static void saveHistory(const playHistory& history);
        static void loadHistory(playHistory& history, const library& lib);
};