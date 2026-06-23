#pragma once
#include <vector>
#include <optional>

#include "config.hpp"
#include "model/track.hpp"

class Library;

struct HistoryNode {
    fs::path     path;
    HistoryNode* prev = nullptr;
    HistoryNode* next = nullptr;
};

class PlayHistory {
    private:
        HistoryNode* head_   = nullptr;
        HistoryNode* tail_   = nullptr;
        HistoryNode* cursor_ = nullptr;
        int          size_   = 0;

    public:
        void push(const Track& t);
        std::optional<fs::path> back();
        std::optional<fs::path> forward();
        std::optional<fs::path> current() const;
        bool canGoBack() const;
        bool canGoForward() const;
        std::vector<fs::path> getHistory() const;
        void clear();
        ~PlayHistory();
};