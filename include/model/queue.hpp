#pragma once
#include <algorithm>
#include <random>
#include <deque>
#include <vector>
#include <optional>
#include <filesystem>
namespace fs = std::filesystem;

#include "track.hpp"

class queue{
    private:
        private:
        std::deque<fs::path>   trackQueue;
        std::deque<fs::path>  originalOrder;
        bool shuffle    = false;
        bool repeat     = false;

    public:
        void load(const std::vector<const track*>& tracks);
        std::optional<fs::path> next();
        void setShuffle(bool enabled);
        void setRepeat(bool enabled);
        std::optional<fs::path> current() const;
        bool hasNext() const;
        void addTrackToFront(const track& t);
        void addTrackToBack(const track& t);
        bool isShuffle() const;
        bool isRepeat() const;
};