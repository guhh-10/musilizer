#pragma once
#include <filesystem>
#include <vector>

struct track{
    std::vector<std::string> artists;
    std::filesystem::path musicpath;
    std::string title;
    int         totalDuration = 0;
};