#include "model/library.hpp"

const std::unordered_map<fs::path, track, PathHash>& library::getTracks() const{
    return tracks;
}

const track* library::findByPath(const fs::path& path) const{
    auto it = tracks.find(fs::weakly_canonical(path));
    if(it != tracks.end()) return &it->second;
    return nullptr;
}

void library::addTrack(track t){
    fs::path key = fs::weakly_canonical(t.getMusicPath());
    tracks.emplace(key, std::move(t));
}
