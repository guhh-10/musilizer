#pragma once
#include <nlohmann/json.hpp>

#include "model/playlist.hpp"
#include "model/play_history.hpp"

class Persistence{
    public:
        static void init();
        static void saveSettings(float volume, bool shuffle, bool repeat);
        static void loadSettings(float& volume, bool& shuffle, bool& repeat);
        static void savePlaylists(const std::vector<Playlist>& playlists);
        static std::vector<Playlist> loadPlaylists(const Library& lib);
        static void saveHistory(const PlayHistory& history);
        static void loadHistory(PlayHistory& history, const Library& lib);
};