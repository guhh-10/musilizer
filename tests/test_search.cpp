#include <catch2/catch_test_macros.hpp>

#include "model/library.hpp"
#include "model/track.hpp"
#include "service/search.hpp"

namespace {

Library buildLib() {
    Library lib;
    lib.addTrack(Track({"Radiohead"},            "a.mp3", "Creep",               3*60+57));
    lib.addTrack(Track({"Radiohead"},            "b.mp3", "Karma Police",        4*60+21));
    lib.addTrack(Track({"Portishead"},           "c.mp3", "Glory Box",           5*60+6 ));
    lib.addTrack(Track({"Massive Attack"},       "d.mp3", "Teardrop",            5*60+29));
    lib.addTrack(Track({"Massive Attack"},       "e.mp3", "Unfinished Sympathy", 5*60+8 ));
    lib.addTrack(Track({"Björk"},                "f.mp3", "Human Behaviour",     3*60+59));
    lib.addTrack(Track({"Björk","Thom Yorke"},   "g.mp3", "Karma Rising",        4*60+0 ));
    return lib;
}

} // namespace

TEST_CASE("Search: empty query returns all tracks", "[search]") {
    auto lib = buildLib();
    Search s(lib);
    auto results = s.query(lib, {});
    CHECK(results.size() == lib.getTracks().size());
}

TEST_CASE("Search: free-text matches title case-insensitively", "[search]") {
    auto lib = buildLib();
    Search s(lib);
    SearchQuery q; q.text = "karma";
    auto results = s.query(lib, q);
    REQUIRE(results.size() == 2);
    for (const auto& r : results)
        CHECK(std::string(r.track->getTitle()).find("Karma") != std::string::npos);
}

TEST_CASE("Search: free-text matches artist", "[search]") {
    auto lib = buildLib();
    Search s(lib);
    SearchQuery q; q.text = "portishead";
    auto results = s.query(lib, q);
    REQUIRE(results.size() == 1);
    CHECK(results.front().track->getTitle() == "Glory Box");
}

TEST_CASE("Search: no results for unknown text", "[search]") {
    auto lib = buildLib();
    Search s(lib);
    SearchQuery q; q.text = "xyzzy_nonexistent";
    CHECK(s.query(lib, q).empty());
}

TEST_CASE("Search: title prefix scores higher than contains", "[search]") {
    auto lib = buildLib();
    Search s(lib);
    SearchQuery q; q.text = "karma";
    auto results = s.query(lib, q);
    REQUIRE(results.size() == 2);
    CHECK(results.front().score == 3.0f);
}

TEST_CASE("Search: artist filter", "[search]") {
    auto lib = buildLib();
    Search s(lib);
    SearchQuery q; q.artistFilter = "Radiohead";
    auto results = s.query(lib, q);
    REQUIRE(results.size() == 2);
    for (const auto& r : results)
        CHECK(r.track->getArtists().front() == "Radiohead");
}

TEST_CASE("Search: artist filter is case-insensitive", "[search]") {
    auto lib = buildLib();
    Search s(lib);
    SearchQuery q; q.artistFilter = "radiohead";
    CHECK(s.query(lib, q).size() == 2);
}

TEST_CASE("Search: multi-artist track matched per artist", "[search]") {
    auto lib = buildLib();
    Search s(lib);

    SearchQuery q1; q1.artistFilter = "Björk";
    CHECK(s.query(lib, q1).size() == 2);

    SearchQuery q2; q2.artistFilter = "Thom Yorke";
    auto r2 = s.query(lib, q2);
    REQUIRE(r2.size() == 1);
    CHECK(r2.front().track->getTitle() == "Karma Rising");
}

TEST_CASE("Search: min duration filter", "[search]") {
    auto lib = buildLib();
    Search s(lib);
    SearchQuery q; q.minDuration = 5 * 60;
    for (const auto& r : s.query(lib, q))
        CHECK(r.track->getDuration() >= 5 * 60);
}

TEST_CASE("Search: max duration filter", "[search]") {
    auto lib = buildLib();
    Search s(lib);
    SearchQuery q; q.maxDuration = 4 * 60;
    for (const auto& r : s.query(lib, q))
        CHECK(r.track->getDuration() <= 4 * 60);
}

TEST_CASE("Search: combined duration range", "[search]") {
    auto lib = buildLib();
    Search s(lib);
    SearchQuery q; q.minDuration = 4*60; q.maxDuration = 5*60;
    for (const auto& r : s.query(lib, q)) {
        CHECK(r.track->getDuration() >= 4 * 60);
        CHECK(r.track->getDuration() <= 5 * 60);
    }
}

TEST_CASE("Search: combined text + artist + duration", "[search]") {
    auto lib = buildLib();
    Search s(lib);
    SearchQuery q; q.text = "karma"; q.artistFilter = "Björk";
    auto results = s.query(lib, q);
    REQUIRE(results.size() == 1);
    CHECK(results.front().track->getTitle() == "Karma Rising");
}

TEST_CASE("Search: sort by title ascending", "[search]") {
    auto lib = buildLib();
    Search s(lib);
    SearchQuery q; q.sortBy = SortField::TITLE; q.sortOrder = SortOrder::ASC;
    auto results = s.query(lib, q);
    for (std::size_t i = 1; i < results.size(); ++i) {
        std::string a = results[i-1].track->getTitle();
        std::string b = results[i  ].track->getTitle();
        std::transform(a.begin(), a.end(), a.begin(), ::tolower);
        std::transform(b.begin(), b.end(), b.begin(), ::tolower);
        CHECK(a <= b);
    }
}

TEST_CASE("Search: sort by duration descending", "[search]") {
    auto lib = buildLib();
    Search s(lib);
    SearchQuery q; q.sortBy = SortField::DURATION; q.sortOrder = SortOrder::DESC;
    auto results = s.query(lib, q);
    for (std::size_t i = 1; i < results.size(); ++i)
        CHECK(results[i-1].track->getDuration() >= results[i].track->getDuration());
}

TEST_CASE("Search: sort by artist ascending", "[search]") {
    auto lib = buildLib();
    Search s(lib);
    SearchQuery q; q.sortBy = SortField::ARTIST; q.sortOrder = SortOrder::ASC;
    auto results = s.query(lib, q);
    for (std::size_t i = 1; i < results.size(); ++i) {
        std::string a = results[i-1].track->getArtists().empty() ? "" : results[i-1].track->getArtists().front();
        std::string b = results[i  ].track->getArtists().empty() ? "" : results[i  ].track->getArtists().front();
        std::transform(a.begin(), a.end(), a.begin(), ::tolower);
        std::transform(b.begin(), b.end(), b.begin(), ::tolower);
        CHECK(a <= b);
    }
}

TEST_CASE("Search: allArtists returns sorted unique names", "[search]") {
    auto lib = buildLib();
    Search s(lib);
    auto artists = s.allArtists();
    CHECK(!artists.empty());
    for (std::size_t i = 1; i < artists.size(); ++i)
        CHECK(artists[i-1] <= artists[i]);
    auto copy = artists;
    copy.erase(std::unique(copy.begin(), copy.end()), copy.end());
    CHECK(copy.size() == artists.size());
}

TEST_CASE("Search: rebuild reflects new tracks", "[search]") {
    auto lib = buildLib();
    Search s(lib);

    SearchQuery q; q.text = "new track";
    CHECK(s.query(lib, q).empty());

    lib.addTrack(Track({"New Artist"}, "h.mp3", "New Track", 200));
    s.rebuild(lib);

    auto results = s.query(lib, q);
    REQUIRE(results.size() == 1);
    CHECK(results.front().track->getTitle() == "New Track");
}