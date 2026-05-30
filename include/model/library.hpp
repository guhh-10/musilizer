#pragma once
#include <unordered_map>
#include <filesystem>
namespace fs = std::filesystem;

#include "track.hpp"

struct PathHash {
    size_t operator()(const fs::path& p) const {
        return fs::hash_value(p);
    }
};

class library {
    private:
        std::unordered_map<fs::path, track, PathHash> tracks;

    public:
        const std::unordered_map<fs::path, track, PathHash>& getTracks() const;
        const track* findByPath(const fs::path& path) const;
        void addTrack(track t);
};