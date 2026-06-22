#pragma once

#include "model/playlist.hpp"
#include "model/track.hpp"
#include <vector>
#include <string>
#include <filesystem>
#include <functional>

namespace fs = std::filesystem;

class PlaylistStore {
private:
    std::vector<Playlist> playlists_;

public:
    std::function<void()> onPlaylistsChanged;

    const std::vector<Playlist>& playlists() const;
    void addPlaylist(Playlist p);
    void removePlaylist(const std::string& name);
    void addTrackToPlaylist(const std::string& playlistName, const Track& track);
    void removeTrackFromPlaylist(const std::string& playlistName, const fs::path& path);
    void moveTrackInPlaylist(const std::string& playlistName, int from, int to);

    void setPlaylists(std::vector<Playlist> p);
};
