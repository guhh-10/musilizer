#pragma once
#include <algorithm>
#include <random>
#include <deque>
#include <vector>
#include <optional>

#include "config.hpp"
#include "model/track.hpp"

class Queue {
    private:
        std::deque<fs::path> track_queue;
        std::deque<fs::path> original_order;
        bool shuffle = false;
        bool repeat  = false;

    public:
        void load(const std::vector<const Track*>& tracks);
        std::optional<fs::path> next();
        void setShuffle(bool enabled);
        void setRepeat(bool enabled);
        std::optional<fs::path> current() const;
        bool hasNext() const;
        void addTrackToFront(const Track& t);
        void addTrackToBack(const Track& t);
        bool isShuffle() const;
        bool isRepeat() const;
};