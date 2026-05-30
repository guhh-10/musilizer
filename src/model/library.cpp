#include "model/library.hpp"

const std::unordered_map<fs::path, track, PathHash>& library::getTracks() const{
    return tracks;
}

const track* library::findByPath(const fs::path& path) const{
    auto it = tracks.find(path.lexically_normal());
    if(it != tracks.end()) return &it->second;
    return nullptr;
}

void library::addTrack(track t){
    fs::path key = t.getMusicPath().lexically_normal();
    tracks.emplace(key, std::move(t));
}
