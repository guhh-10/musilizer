#include "controller/search_controller.hpp"

SearchController::SearchController(const Library& lib)
    : lib_(lib), search_(lib) {}

void SearchController::rebuild() {
    search_.rebuild(lib_);
}

std::vector<SearchResult> SearchController::query(const std::string& text) const {
    SearchQuery q;
    q.text = text;
    return search_.query(lib_, q);
}

std::vector<SearchResult> SearchController::query(const SearchQuery& q) const {
    return search_.query(lib_, q);
}

void SearchController::setText(const std::string& text) {
    last_query_.text = text;
}

void SearchController::setArtistFilter(const std::string& artist) {
    last_query_.artistFilter = artist;
}

void SearchController::setDurationRange(int minSec, int maxSec) {
    last_query_.minDuration = minSec;
    last_query_.maxDuration = maxSec;
}

void SearchController::setSortField(SortField field) {
    last_query_.sortBy = field;
}

void SearchController::setSortOrder(SortOrder order) {
    last_query_.sortOrder = order;
}

void SearchController::clearFilters() {
    last_query_ = {};
}

std::vector<SearchResult> SearchController::requery() const {
    return search_.query(lib_, last_query_);
}

std::vector<std::string> SearchController::allArtists() const {
    return search_.allArtists();
}

const SearchQuery& SearchController::lastQuery() const {
    return last_query_;
}