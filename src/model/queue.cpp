#include <deque>

#include "model/queue.hpp"

void queue::load(const std::vector<track>& tracks){
    trackQueue.assign(tracks.begin(), tracks.end());
}

const track& queue::next(){
    if(hasNext())
        trackQueue.pop_front();
    return trackQueue.front();
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