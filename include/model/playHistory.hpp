#pragma once
#include <vector>
#include <optional>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

#include "track.hpp"

class library;

class playHistory {
    private:
        std::vector<fs::path> history;
        int cursor = -1;

    public:
        void push(const track& t);
        std::optional<fs::path> back();
        std::optional<fs::path> forward();
        std::optional<fs::path> current() const;
        bool canGoBack() const;
        bool canGoForward() const;
        const std::vector<fs::path>& getHistory() const;
        void clear();
};