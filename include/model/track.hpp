#pragma once
#include <string>
#include <vector>

#include "config.hpp"

class Track{
    private:
        std::vector<std::string> artists;
        std::vector<std::string> genres;
        fs::path music_path;
        std::string title;
        int total_duration = 0;

    public:
        // Full constructor (artists, path, title, duration, genres)
        Track(std::vector<std::string> artists,
              fs::path music_path,
              std::string title,
              int duration,
              std::vector<std::string> genres = {})
            : artists(std::move(artists)),
              genres(std::move(genres)),
              music_path(std::move(music_path)),
              title(std::move(title)),
              total_duration(duration) {}

        const std::vector<std::string>& getArtists()  const { return artists; }
        const std::vector<std::string>& getGenres()   const { return genres; }
        const std::string&              getTitle()    const { return title; }
        const fs::path&                 getMusicPath()const { return music_path; }
        int                             getDuration() const { return total_duration; }
};