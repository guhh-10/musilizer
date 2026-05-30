#include <deque>

#include "model/queue.hpp"

void queue::load(const std::vector<const track*>& tracks){
    trackQueue.clear();
    originalOrder.clear();
    for(const track* t : tracks){
        trackQueue.push_back(t->getMusicPath());
        originalOrder.push_back(t->getMusicPath());
    }
}

std::optional<fs::path> queue::next(){
    if(trackQueue.empty())
        return std::nullopt;

    if(!hasNext()){
        if(repeat) return trackQueue.front();
        return std::nullopt;
    }

    fs::path looping = trackQueue.front();
    if(repeat){
        trackQueue.push_back(looping);
        originalOrder.push_back(looping);
    }
    trackQueue.pop_front();
    return trackQueue.front();
}

void queue::setShuffle(bool enabled){
    if(trackQueue.empty()) {
        shuffle = enabled;
        return;
    }

    shuffle = enabled;
    if(shuffle){
        originalOrder.assign(trackQueue.begin(), trackQueue.end());
        std::shuffle(trackQueue.begin() + 1, trackQueue.end(),
                     std::mt19937{std::random_device{}()});
    } else {
        fs::path current = trackQueue.front();
        trackQueue.assign(originalOrder.begin(), originalOrder.end());
        auto it = std::find(trackQueue.begin(), trackQueue.end(), current);
        if(it != trackQueue.end())
            std::rotate(trackQueue.begin(), it, it + 1);
    }
}

void queue::setRepeat(bool enabled){
    repeat = enabled;
}

std::optional<fs::path> queue::current() const{
    if(trackQueue.empty())
        return std::nullopt;
    return trackQueue.front();
}

bool queue::hasNext() const {
    return trackQueue.size() > 1;
}

void queue::addTrackToFront(const track& t){
    fs::path path = t.getMusicPath();
    if(trackQueue.empty()){
        trackQueue.push_front(path);
        originalOrder.push_front(path);
    } else {
        trackQueue.insert(trackQueue.begin() + 1, path);
        originalOrder.insert(originalOrder.begin() + 1, path);
    }
}

void queue::addTrackToBack(const track& t){
    fs::path path = t.getMusicPath();
    trackQueue.push_back(path);
    originalOrder.push_back(path);
}

bool queue::isShuffle() const{
    return shuffle; 
}

bool queue::isRepeat() const{
    return repeat;
}