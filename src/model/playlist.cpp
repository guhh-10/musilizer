#include "model/playlist.hpp"

playlist::playlist(std::string name) : name(std::move(name)) {}

void playlist::addTrack(const track& t){
    playlistTracks.push_back(t);
}

void playlist::removeTrack(int index){
    if (index < 0 || index >= size()) return;
    playlistTracks.erase(playlistTracks.begin() + index);
}

const track& playlist::getTrack(int index) const{
    return playlistTracks.at(index);
}

const std::vector<track>& playlist::getPlaylistTracks() const{
    return playlistTracks;
}

void playlist::moveTrack(int from, int to){
    if (from == to) return;
    if (from < 0 || from >= size()) return;
    if (to < 0 || to >= size()) return;
    
    track temp = playlistTracks[from];
    playlistTracks.erase(playlistTracks.begin() + from);
    playlistTracks.insert(playlistTracks.begin() + to, temp);
}

void playlist::clear(){
    playlistTracks.clear();
}

const std::string& playlist::getName() const{
    return name;
}

int playlist::size() const{
    return static_cast<int>(playlistTracks.size());
}

bool playlist::isEmpty() const{
    return playlistTracks.empty();
}

int playlist::getTotalDuration() const{
    int total = 0;

    for (auto& track : playlistTracks){
        total += track.getDuration();
    }

    return total;
}