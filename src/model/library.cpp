#include <vector>

#include "model/library.hpp"

void library::addTrack(track t){
    tracks.push_back(std::move(t));
}