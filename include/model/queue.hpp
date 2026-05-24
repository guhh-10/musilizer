#pragma once
#include <deque>

#include "track.hpp"

class queue{
    private:
        std::deque<track> trackQueue;

    public:
        void load(const std::vector<track>& tracks);
        const track& next();
        const track& current() const;
        bool hasNext() const;
        void addTrackToFront(const track& t);
        void addTrackToBack(const track& t);
};