#include "model/library.hpp"

const std::unordered_map<fs::path, Track, PathHash>& Library::getTracks() const {
    return tracks;
}

const Track* Library::findByPath(const fs::path& path) const {
    auto it = tracks.find(path.lexically_normal());
    if (it != tracks.end()) return &it->second;
    return nullptr;
}

void Library::addTrack(Track t) {
    fs::path key = t.getMusicPath().lexically_normal();
    tracks.emplace(key, std::move(t));
}
