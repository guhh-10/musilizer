#pragma once
#include <vector>

#include "track.hpp"

class playHistory {
    private:
        std::vector<track> history;
        int cursor = -1;

    public:
        void push(const track& t);
        const track& back();
        const track& forward();
        const track& current() const;
        bool canGoBack() const;
        bool canGoForward() const;
        const std::vector<track>& getHistory() const;
        void clear();
};