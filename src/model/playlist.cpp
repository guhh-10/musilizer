#include <algorithm>

#include "model/playlist.hpp"
#include "model/library.hpp"

playlist::playlist(std::string name) : name(std::move(name)) {}

void playlist::addTrack(const track& t){
    trackPaths.push_back(t.getMusicPath().string());
}

void playlist::removeTrack(const fs::path& path){
    auto it = std::find(trackPaths.begin(), trackPaths.end(), path.string());
    if (it != trackPaths.end())
        trackPaths.erase(it);
}


const std::string& playlist::getTrack(int index) const{
    return trackPaths.at(index);
}

const std::vector<std::string>& playlist::getPlaylistTracks() const{
    return trackPaths;
}

void playlist::moveTrack(int from, int to){
    if (from == to) return;
    if (from < 0 || from >= size()) return;
    if (to < 0 || to >= size()) return;
    
    std::string temp = trackPaths[from];
    trackPaths.erase(trackPaths.begin() + from);
    trackPaths.insert(trackPaths.begin() + to, temp);
}

void playlist::clear(){
    trackPaths.clear();
}

const std::string& playlist::getName() const{
    return name;
}

int playlist::size() const{
    return static_cast<int>(trackPaths.size());
}

bool playlist::isEmpty() const{
    return trackPaths.empty();
}

int playlist::getTotalDuration(const library& lib) const{
    int total = 0;
    for (const auto& path : trackPaths){
        const track* t = lib.findByPath(path);
        if (t)
            total += t->getDuration();
    }
    return total;
}

std::vector<const track*> playlist::resolve(const library& lib) const{
    std::vector<const track*> result;
    for (const auto& path : trackPaths) {
        const track* t = lib.findByPath(path);
        if (t)
            result.push_back(t);
    }
    return result;

}