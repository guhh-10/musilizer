#pragma once
#include <algorithm>
#include <random>
#include <deque>
#include <vector>

#include "track.hpp"

class queue{
    private:
        std::deque<std::string> trackQueue;
        std::vector<std::string> originalOrder;
        bool shuffle = false;

    public:
        void load(const std::vector<const track*>& tracks);
        const std::string& next();
        void setShuffle(bool enabled);
        const std::string& current() const;
        bool hasNext() const;
        void addTrackToFront(const track& t);
        void addTrackToBack(const track& t);
        bool isShuffle() const;
};