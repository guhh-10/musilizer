#include <deque>

#include "model/queue.hpp"

void queue::load(const std::vector<const track*>& tracks){
    trackQueue.clear();
    originalOrder.clear();
    for(const track* t : tracks){
        trackQueue.push_back(t->getMusicPath().string());
        originalOrder.push_back(t->getMusicPath().string());
    }
}

const std::string& queue::next(){
    if(!hasNext())
        return trackQueue.front();

    if(repeat){
        trackQueue.push_back(trackQueue.front());
        if(shuffle) originalOrder.push_back(trackQueue.front());
    }

    trackQueue.pop_front();
    return trackQueue.front();
}

void queue::setShuffle(bool enabled){
    shuffle = enabled;
    if (shuffle){
        originalOrder.assign(trackQueue.begin(), trackQueue.end());
        std::shuffle(trackQueue.begin() + 1, trackQueue.end(), 
                     std::mt19937{std::random_device{}()});
    } else{
        std::string current = trackQueue.front();
        trackQueue.assign(originalOrder.begin(), originalOrder.end());
        auto it = std::find(trackQueue.begin(), trackQueue.end(), current);
        if (it != trackQueue.end()){
            std::rotate(trackQueue.begin(), it, it + 1);
        }
    }
}

void queue::setRepeat(bool enabled){
    repeat = enabled;
}

const std::string& queue::current() const{
    return trackQueue.front();
}

bool queue::hasNext() const {
    return trackQueue.size() > 1; // >1 because front is current
}

void queue::addTrackToFront(const track& t){
    if (trackQueue.empty())
        trackQueue.push_front(t.getMusicPath().string());
    else
        trackQueue.insert(trackQueue.begin() + 1, t.getMusicPath().string());
}

void queue::addTrackToBack(const track& t){
    trackQueue.push_back(t.getMusicPath().string());
}

bool queue::isShuffle() const{
    return shuffle; 
}

bool queue::isRepeat() const{
    return repeat;
}