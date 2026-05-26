#pragma once
#include <vector>
#include <string>

#include "track.hpp"

class playlist{
    private:
        std::string         name;
        std::vector<track>  playlistTracks;

    public:
        playlist(std::string name);

        void addTrack(const track& t);
        void removeTrack(int index);
        const track& getTrack(int index) const;
        const std::vector<track>& getPlaylistTracks() const;
        void moveTrack(int from, int to);
        void clear();
        const std::string& getName() const;
        int size() const;
        bool isEmpty() const;
        int getTotalDuration() const;
};