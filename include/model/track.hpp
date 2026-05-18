#pragma once
#include <filesystem>
#include <vector>

class track{
    private:
        std::vector<std::string> artists;
        std::filesystem::path musicpath;
        std::string title;
        int         totalDuration = 0;
    
    public:
        track(std::vector<std::string> a, std::filesystem::path m, std::string t, int d)
            : artists(std::move(a)), musicpath(std::move(m)), title(std::move(t)), totalDuration(d) {}

        const std::vector<std::string>& getArtists() const { return artists; }
        const std::string& getTitle() const { return title; }
        const std::filesystem::path& getMusicPath() const { return musicpath; }
        int getDuration() const { return totalDuration; }
};