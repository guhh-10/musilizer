#include <vector>

#include "model/library.hpp"

void library::addTrack(track track){
    tracks.push_back(std::move(track));
}