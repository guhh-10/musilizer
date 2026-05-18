#pragma once
#include <vector>

#include "track.hpp"

class library{
    private:
        std::vector<track> tracks;
    public:
        const std::vector<track>& getTracks() const { return tracks; }
    
        void addTrack(track track);
};