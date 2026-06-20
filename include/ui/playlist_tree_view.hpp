#pragma once

#include <functional>
#include <string>
#include <vector>

#include "config.hpp"
#include "model/playlist.hpp"

struct PlaylistItemTrack {
    std::string name;
    std::string duration;
};

struct PlaylistGroup {
    std::string name;
    std::vector<PlaylistItemTrack> tracks;
};

struct PlaylistViewActions {
    std::function<void(int playlistIndex)> playPlaylist;
    std::function<void(int playlistIndex, int trackIndex)> playTrack;
    std::function<void(const std::string& name)> removePlaylist;
    std::function<void(const std::string& name, int from, int to)> moveTrack;
    std::function<void(const std::string& name, const fs::path& path)> removeTrack;
    std::function<const std::vector<Playlist>&()> getPlaylists;
};

void StrictTwoTierPlaylistView(const char* str_id, const std::vector<PlaylistGroup>& list, const PlaylistViewActions& actions);
