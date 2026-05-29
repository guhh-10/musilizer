#pragma once
#include <unordered_map>

#include "track.hpp"

class library{
    private:
        std::unordered_map<std::string, track> tracks;

    public:
        const std::unordered_map<std::string, track>& getTracks() const;
        const track* findByPath(const std::string& path) const;

        void addTrack(track t);
};