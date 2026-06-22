#include "model/playlist_store.hpp"
#include <algorithm>

const std::vector<Playlist>& PlaylistStore::playlists() const {
    return playlists_;
}

void PlaylistStore::addPlaylist(Playlist p) {
    playlists_.push_back(std::move(p));
    if (onPlaylistsChanged) onPlaylistsChanged();
}

void PlaylistStore::removePlaylist(const std::string& name) {
    auto it = std::find_if(playlists_.begin(), playlists_.end(),
        [&](const Playlist& p) { return p.getName() == name; });
    if (it != playlists_.end()) {
        playlists_.erase(it);
        if (onPlaylistsChanged) onPlaylistsChanged();
    }
}

void PlaylistStore::addTrackToPlaylist(const std::string& name, const Track& track) {
    auto it = std::find_if(playlists_.begin(), playlists_.end(),
        [&](const Playlist& p) { return p.getName() == name; });
    if (it != playlists_.end()) {
        it->addTrack(track);
        if (onPlaylistsChanged) onPlaylistsChanged();
    }
}

void PlaylistStore::removeTrackFromPlaylist(const std::string& name, const fs::path& path) {
    auto it = std::find_if(playlists_.begin(), playlists_.end(),
        [&](const Playlist& p) { return p.getName() == name; });
    if (it != playlists_.end()) {
        it->removeTrack(path);
        if (onPlaylistsChanged) onPlaylistsChanged();
    }
}

void PlaylistStore::moveTrackInPlaylist(const std::string& name, int from, int to) {
    auto it = std::find_if(playlists_.begin(), playlists_.end(),
        [&](const Playlist& p) { return p.getName() == name; });
    if (it != playlists_.end()) {
        it->moveTrack(from, to);
        if (onPlaylistsChanged) onPlaylistsChanged();
    }
}

void PlaylistStore::setPlaylists(std::vector<Playlist> p) {
    playlists_ = std::move(p);
    if (onPlaylistsChanged) onPlaylistsChanged();
}
