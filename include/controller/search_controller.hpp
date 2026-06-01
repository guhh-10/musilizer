#pragma once
#include <string>
#include <vector>

#include "model/library.hpp"
#include "service/search.hpp"

class SearchController {
    private:
        const Library& lib_;
        Search         search_;
        SearchQuery    last_query_;

    public:
        explicit SearchController(const Library& lib);

        void rebuild();

        std::vector<SearchResult> query(const std::string& text) const;

        std::vector<SearchResult> query(const SearchQuery& q) const;

        void setText(const std::string& text);
        void setArtistFilter(const std::string& artist);
        void setDurationRange(int minSec, int maxSec);
        void setSortField(SortField field);
        void setSortOrder(SortOrder order);
        void clearFilters();

        std::vector<SearchResult> requery() const;

        // List of all indexed artist names (sorted)
        std::vector<std::string> allArtists() const;

        const SearchQuery& lastQuery() const;
};