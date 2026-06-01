#pragma once
#include <string>
#include <vector>
#include <map>
#include <set>

#include "model/library.hpp"
#include "model/track.hpp"

enum class SortField{ 
    TITLE, 
    ARTIST, 
    DURATION 
};

enum class SortOrder{ 
    ASC, 
    DESC
};

struct SearchQuery{
    std::string text;
    std::string artistFilter;
    int         minDuration = 0;
    int         maxDuration = 0;
    SortField   sortBy    = SortField::TITLE;
    SortOrder   sortOrder = SortOrder::ASC;
};

struct SearchResult{
    const Track* track;
    float        score;
};

class Search{
    private:
        std::map<std::string, std::set<fs::path>> title_index;
        std::map<std::string, std::set<fs::path>> artist_index;

        static std::vector<std::string> tokenize(const std::string& s);
        float scoreTrack(const Track& t, const std::string& lowerText) const;

    public:
        explicit Search(const Library& lib);
        void rebuild(const Library& lib);
        std::vector<SearchResult> query(const Library& lib, const SearchQuery& q) const;
        std::vector<std::string> allArtists() const;
};