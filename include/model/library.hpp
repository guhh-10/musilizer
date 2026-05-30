#pragma once
#include <unordered_map>

#include "config.hpp"
#include "model/track.hpp"

struct PathHash {
    size_t operator()(const fs::path& p) const {
        return fs::hash_value(p);
    }
};

class Library {
    private:
        std::unordered_map<fs::path, Track, PathHash> tracks;

    public:
        const std::unordered_map<fs::path, Track, PathHash>& getTracks() const;
        const Track* findByPath(const fs::path& path) const;
        void addTrack(Track t);
};