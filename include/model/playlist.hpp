#pragma once
#include <vector>
#include <string>

#include "config.hpp"
#include "model/track.hpp"

class Library;

class Playlist{
    private:
        std::string           name;
        std::vector<fs::path> track_paths;   // was vector<string>

    public:
        explicit Playlist(std::string name);

        void addTrack(const Track& t);
        void removeTrack(const fs::path& path);
        const fs::path& getTrack(int index) const;              // was string
        const std::vector<fs::path>& getPlaylistTracks() const; // was vector<string>
        void moveTrack(int from, int to);
        void clear();
        const std::string& getName() const;
        int size() const;
        bool isEmpty() const;
        int getTotalDuration(const Library& lib) const;
        std::vector<const Track*> resolve(const Library& lib) const;
};