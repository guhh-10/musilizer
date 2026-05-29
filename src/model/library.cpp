#include "model/library.hpp"

const std::unordered_map<std::string, track>& library::getTracks() const{
    return tracks;
}

const track* library::findByPath(const std::string& path) const{
    auto it = tracks.find(path);
    if (it != tracks.end())
        return &it->second;
    return nullptr;
}

void library::addTrack(track t){
    std::string key = t.getMusicPath().string();
    tracks[key] = std::move(t);
}