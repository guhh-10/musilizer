#include "model/play_history.hpp"
#include "config.hpp"

void PlayHistory::push(const Track& t){
    if (cursor < static_cast<int>(history.size()) - 1)
        history.erase(history.begin() + cursor + 1, history.end());

    history.push_back(t.getMusicPath());
    cursor = static_cast<int>(history.size()) - 1;

    if(static_cast<int>(history.size()) > config::MAX_HISTORY){
        history.erase(history.begin());
        cursor--;
    }
}

std::optional<fs::path> PlayHistory::back(){
    if(history.empty() || !canGoBack())
        return std::nullopt;
    cursor--;
    return history[cursor];
}

std::optional<fs::path> PlayHistory::forward(){
    if(history.empty() || !canGoForward())
        return std::nullopt;
    cursor++;
    return history[cursor];
}

std::optional<fs::path> PlayHistory::current() const{
    if(history.empty() || cursor < 0)
        return std::nullopt;
    return history[cursor];
}

bool PlayHistory::canGoBack() const{
    return cursor > 0;
}

bool PlayHistory::canGoForward() const{
    return cursor < static_cast<int>(history.size()) - 1;
}

const std::vector<fs::path>& PlayHistory::getHistory() const{
    return history;
}

void PlayHistory::clear(){
    history.clear();
    cursor = -1;
}