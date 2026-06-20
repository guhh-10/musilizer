#pragma once

#include <functional>
#include <vector>

#include "model/track.hpp"

struct QueueViewActions {
    std::function<void(std::size_t index)> playIndex;
    std::function<void(std::size_t index)> moveUp;
    std::function<void(std::size_t index)> moveDown;
    std::function<void(std::size_t index)> remove;
};

void RenderQueueTable(const std::vector<const Track*>& tracks, const Track* currentTrack, const QueueViewActions& actions);
