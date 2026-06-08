#include <deque>
#include <algorithm>
#include <random>

#include "model/queue.hpp"

void Queue::load(const std::vector<const Track*>& tracks) {
    track_queue.clear();
    original_order.clear();
    for (const Track* t : tracks) {
        track_queue.push_back(t->getMusicPath());
        original_order.push_back(t->getMusicPath());
    }
}

std::optional<fs::path> Queue::next() {
    if (track_queue.empty())
        return std::nullopt;

    // Pop the track that just completed playing
    track_queue.pop_front();

    // If the active playback queue is now empty
    if (track_queue.empty()) {
        if (!repeat) {
            // Repeat is OFF: keep it clear so fallback recommenders can take over
            return std::nullopt;
        }

        // Repeat is ON: Restock from our original order playlist blueprint
        track_queue.assign(original_order.begin(), original_order.end());
        
        if (shuffle && track_queue.size() > 1) {
            std::shuffle(track_queue.begin(), track_queue.end(),
                         std::mt19937{std::random_device{}()});
        }
    }

    if (track_queue.empty())
        return std::nullopt;

    return track_queue.front();
}

void Queue::setShuffle(bool enabled) {
    if (track_queue.empty()) {
        shuffle = enabled;
        return;
    }

    shuffle = enabled;
    if (shuffle) {
        original_order.assign(track_queue.begin(), track_queue.end());
        // Mix everything behind the currently playing track
        if (track_queue.size() > 1) {
            std::shuffle(track_queue.begin() + 1, track_queue.end(),
                         std::mt19937{std::random_device{}()});
        }
    } else {
        fs::path current = track_queue.front();
        track_queue.assign(original_order.begin(), original_order.end());
        auto it = std::find(track_queue.begin(), track_queue.end(), current);
        if (it != track_queue.end())
            std::rotate(track_queue.begin(), it, track_queue.end());
    }
}

void Queue::setRepeat(bool enabled) {
    repeat = enabled;
}

std::optional<fs::path> Queue::current() const {
    if (track_queue.empty())
        return std::nullopt;
    return track_queue.front();
}

bool Queue::hasNext() const {
    return track_queue.size() > 1;
}

void Queue::addTrackToFront(const Track& t) {
    fs::path path = t.getMusicPath();
    if (track_queue.empty()) {
        track_queue.push_front(path);
        original_order.push_front(path);
    } else {
        track_queue.insert(track_queue.begin() + 1, path);
        original_order.insert(original_order.begin() + 1, path);
    }
}

void Queue::addTrackToBack(const Track& t) {
    fs::path path = t.getMusicPath();
    track_queue.push_back(path);
    original_order.push_back(path);
}

bool Queue::isShuffle() const {
    return shuffle;
}

bool Queue::isRepeat() const {
    return repeat;
}