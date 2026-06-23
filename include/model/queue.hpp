#pragma once
#include <algorithm>
#include <random>
#include <vector>
#include <optional>
#include <filesystem>

#include "config.hpp"
#include "model/track.hpp"

namespace fs = std::filesystem;

struct QueueNode {
    fs::path   path;
    QueueNode* next = nullptr;
};

class Queue {
    private:
        QueueNode* head_          = nullptr;
        QueueNode* tail_          = nullptr;
        QueueNode* origin_head_   = nullptr;
        QueueNode* origin_tail_   = nullptr;
        int        size_          = 0;
        bool       shuffle_       = false;
        bool       repeat_        = false;

        void clearQueue();
        void clearOrigin();
        void rebuildFromOrigin();
        void makeCircularIfRepeat();

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

        std::vector<fs::path> snapshot() const;
        ~Queue();
};