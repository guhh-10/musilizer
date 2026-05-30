#pragma once
#include <vector>
#include <optional>

#include "config.hpp"
#include "model/track.hpp"

class Library;

class PlayHistory {
    private:
        std::vector<fs::path> history;
        int cursor = -1;

    public:
        void push(const Track& t);
        std::optional<fs::path> back();
        std::optional<fs::path> forward();
        std::optional<fs::path> current() const;
        bool canGoBack() const;
        bool canGoForward() const;
        const std::vector<fs::path>& getHistory() const;
        void clear();
};