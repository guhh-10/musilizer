// playHistory.cpp
#include "model/playHistory.hpp"

void playHistory::push(const track& t){
    if (cursor < static_cast<int>(history.size()) - 1)
        history.erase(history.begin() + cursor + 1, history.end());

    history.push_back(t);
    cursor = static_cast<int>(history.size()) - 1;
}

const track& playHistory::back(){
    if (canGoBack())
        cursor--;
    return history[cursor];
}

const track& playHistory::forward(){
    if (canGoForward())
        cursor++;
    return history[cursor];
}

const track& playHistory::current() const{
    return history[cursor];
}

bool playHistory::canGoBack() const{
    return cursor > 0;
}

bool playHistory::canGoForward() const{
    return cursor < static_cast<int>(history.size()) - 1;
}

const std::vector<track>& playHistory::getHistory() const{
    return history;
}

void playHistory::clear(){
    history.clear();
    cursor = -1;
}