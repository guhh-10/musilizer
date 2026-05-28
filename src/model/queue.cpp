#include <deque>

#include "model/queue.hpp"

void queue::load(const std::vector<track>& tracks){
    trackQueue.assign(tracks.begin(), tracks.end());
    originalOrder.assign(tracks.begin(), tracks.end());
}

const track& queue::next(){
    if(hasNext())
        trackQueue.pop_front();
    return trackQueue.front();
}

void queue::setShuffle(bool enabled){
    shuffle = enabled;
    if (shuffle) {
        originalOrder.assign(trackQueue.begin(), trackQueue.end());
        std::shuffle(trackQueue.begin() + 1, trackQueue.end(), 
                     std::mt19937{std::random_device{}()});
    } else {
        track current = trackQueue.front();
        trackQueue.assign(originalOrder.begin(), originalOrder.end());
        auto it = std::find_if(trackQueue.begin(), trackQueue.end(),
            [&current](const track& t){ return t.getTitle() == current.getTitle(); });
        if (it != trackQueue.end()) {
            std::rotate(trackQueue.begin(), it, it + 1);
        }
    }
}

const track& queue::current() const{
    return trackQueue.front();
}

bool queue::hasNext() const {
    return trackQueue.size() > 1; // >1 because front is current
}

void queue::addTrackToFront(const track& t){
    if (trackQueue.empty())
        trackQueue.push_front(t);
    else
        trackQueue.insert(trackQueue.begin() + 1, t);
}

void queue::addTrackToBack(const track& t){
    trackQueue.push_back(t);
}

bool queue::isShuffle() const{
    return shuffle; 
}