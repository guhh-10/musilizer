#include <algorithm>
#include "model/playlist.hpp"
#include "model/library.hpp"

Playlist::Playlist(std::string name) : name(std::move(name)) {}

void Playlist::addTrack(const Track& t) {
    track_paths.push_back(t.getMusicPath());
}

void Playlist::removeTrack(const fs::path& path) {
    auto it = std::find(track_paths.begin(), track_paths.end(), path);
    if (it != track_paths.end())
        track_paths.erase(it);
}

const fs::path& Playlist::getTrack(int index) const {
    return track_paths.at(index);
}

const std::vector<fs::path>& Playlist::getPlaylistTracks() const {
    return track_paths;
}

void Playlist::moveTrack(int from, int to) {
    if (from == to) return;
    if (from < 0 || from >= size()) return;
    if (to < 0 || to >= size()) return;

    auto f = static_cast<std::ptrdiff_t>(from);
    auto t = static_cast<std::ptrdiff_t>(to);

    fs::path temp = track_paths[f];
    track_paths.erase(track_paths.begin() + f);
    track_paths.insert(track_paths.begin() + t, temp);
}

void Playlist::clear() {
    track_paths.clear();
}

const std::string& Playlist::getName() const {
    return name;
}

int Playlist::size() const {
    return static_cast<int>(track_paths.size());
}

bool Playlist::isEmpty() const {
    return track_paths.empty();
}

int Playlist::getTotalDuration(const Library& lib) const {
    int total = 0;
    for (const auto& path : track_paths) {
        const Track* t = lib.findByPath(path);
        if (t) total += t->getDuration();
    }
    return total;
}

std::vector<const Track*> Playlist::resolve(const Library& lib) const {
    std::vector<const Track*> result;
    for (const auto& path : track_paths) {
        const Track* t = lib.findByPath(path);
        if (t) result.push_back(t);
    }
    return result;
}