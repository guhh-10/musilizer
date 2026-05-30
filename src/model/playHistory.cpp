// playHistory.cpp
#include "model/playHistory.hpp"
#include "config.hpp"

void playHistory::push(const track& t){
    if (cursor < static_cast<int>(history.size()) - 1)
        history.erase(history.begin() + cursor + 1, history.end());

    history.push_back(t.getMusicPath());
    cursor = static_cast<int>(history.size()) - 1;

    if(static_cast<int>(history.size()) > config::MAX_HISTORY){
        history.erase(history.begin());
        cursor--;
    }
}

std::optional<fs::path> playHistory::back(){
    if(history.empty() || !canGoBack())
        return std::nullopt;
    cursor--;
    return history[cursor];
}

std::optional<fs::path> playHistory::forward(){
    if(history.empty() || !canGoForward())
        return std::nullopt;
    cursor++;
    return history[cursor];
}

std::optional<fs::path> playHistory::current() const{
    if(history.empty() || cursor < 0)
        return std::nullopt;
    return history[cursor];
}

bool playHistory::canGoBack() const{
    return cursor > 0;
}

bool playHistory::canGoForward() const{
    return cursor < static_cast<int>(history.size()) - 1;
}

const std::vector<fs::path>& playHistory::getHistory() const{
    return history;
}

void playHistory::clear(){
    history.clear();
    cursor = -1;
}