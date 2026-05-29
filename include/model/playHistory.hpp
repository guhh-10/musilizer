#pragma once
#include <vector>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

#include "track.hpp"

class library;

class playHistory {
    private:
        std::vector<std::string> history;
        int cursor = -1;

    public:
        void push(const track& t);
        const std::string& back();
        const std::string& forward();
        const std::string& current() const;
        bool canGoBack() const;
        bool canGoForward() const;
        const std::vector<std::string>& getHistory() const;
        void clear();
};