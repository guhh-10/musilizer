#pragma once
#include <algorithm>
#include <random>
#include <deque>

#include "track.hpp"

class queue{
    private:
        std::deque<track> trackQueue;
        std::vector<track> originalOrder;
        bool shuffle = false;

    public:
        void load(const std::vector<track>& tracks);
        const track& next();
        void setShuffle(bool enabled);
        const track& current() const;
        bool hasNext() const;
        void addTrackToFront(const track& t);
        void addTrackToBack(const track& t);
        bool isShuffle() const;
};