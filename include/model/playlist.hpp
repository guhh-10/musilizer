#pragma once
#include <vector>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

#include "track.hpp"

class library; 

class playlist{
    private:
        std::string         name;
        std::vector<std::string>  trackPaths;

    public:
        playlist(std::string name);

        void addTrack(const track& t);
        void removeTrack(const fs::path& path);
        const std::string& getTrack(int index) const;
        const std::vector<std::string>& getPlaylistTracks() const;
        void moveTrack(int from, int to);
        void clear();
        const std::string& getName() const;
        int size() const;
        bool isEmpty() const;
        int getTotalDuration(const library& lib) const;
        std::vector<const track*> resolve(const library& lib) const;
};