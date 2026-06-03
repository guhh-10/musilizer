#include <catch2/catch_test_macros.hpp>

#include "model/library.hpp"
#include "model/track.hpp"
#include "service/recommender.hpp"

namespace {

Library buildLib() {
    Library lib;
    lib.addTrack(Track({"Band A"}, "1.mp3", "A Rock Song", 200, {"rock", "alternative"}));
    lib.addTrack(Track({"Band B"}, "2.mp3", "A Metal Song", 200, {"metal"}));
    lib.addTrack(Track({"Band C"}, "3.mp3", "A Pop Song", 200, {"pop"}));
    lib.addTrack(Track({"Band D"}, "4.mp3", "A Folk Song", 200, {"folk"}));
    lib.addTrack(Track({"Band E"}, "5.mp3", "Another Rock Song", 200, {"rock"}));
    return lib;
}

} // namespace

TEST_CASE("Recommender: default graph has expected weights", "[recommender]") {
    GenreGraph g = Recommender::buildDefaultGraph();
    CHECK(g.weight("rock", "alternative") > 0.0f);
    CHECK(g.weight("rock", "metal") > 0.0f);
    CHECK(g.weight("pop", "metal") == 0.0f); // Default shouldn't link these directly
}

TEST_CASE("Recommender: recommends similar tracks based on genre", "[recommender]") {
    auto lib = buildLib();
    Recommender rec(Recommender::buildDefaultGraph());
    
    const Track* seed = lib.findByPath("1.mp3"); // Rock, Alternative
    REQUIRE(seed != nullptr);
    
    auto results = rec.recommend(lib, *seed);
    
    // It should exclude the seed itself
    for (const auto& r : results) {
        CHECK(r.track->getMusicPath() != "1.mp3");
    }
    
    // We expect "Another Rock Song" and "A Metal Song" to be matched
    REQUIRE(!results.empty());
    
    bool foundRock = false;
    for (const auto& r : results) {
        if (r.track->getMusicPath() == "5.mp3") foundRock = true;
    }
    CHECK(foundRock);
}

TEST_CASE("Recommender: zero-score candidates are excluded if there are better matches", "[recommender]") {
    Library lib;
    lib.addTrack(Track({"Band A"}, "1.mp3", "Seed Metal", 200, {"metal"}));
    lib.addTrack(Track({"Band B"}, "2.mp3", "Good Metal", 200, {"metal"}));
    lib.addTrack(Track({"Band C"}, "3.mp3", "No Match Classical", 200, {"classical"}));
    
    Recommender rec(Recommender::buildDefaultGraph());
    const Track* seed = lib.findByPath("1.mp3");
    
    auto results = rec.recommend(lib, *seed);
    
    // There is a match (2.mp3). So 3.mp3 (0 overlap) should be excluded.
    REQUIRE(results.size() == 1);
    CHECK(results[0].track->getMusicPath() == "2.mp3");
}

TEST_CASE("Recommender: unranked fallback if nothing has overlap", "[recommender]") {
    Library lib;
    lib.addTrack(Track({"Band A"}, "1.mp3", "A Metal Song", 200, {"metal"}));
    lib.addTrack(Track({"Band B"}, "2.mp3", "A Classical Song", 200, {"classical"}));
    
    Recommender rec(Recommender::buildDefaultGraph());
    const Track* seed = lib.findByPath("1.mp3");
    
    auto results = rec.recommend(lib, *seed);
    
    // Nothing has overlap. So anyScored is false.
    // Thus it returns all other tracks (the 2.mp3) with score 0.0f
    REQUIRE(results.size() == 1);
    CHECK(results[0].track->getMusicPath() == "2.mp3");
    CHECK(results[0].score == 0.0f);
}

TEST_CASE("Recommender: recommendByGenres allows cross-genre recommendations without seed track", "[recommender]") {
    auto lib = buildLib();
    Recommender rec(Recommender::buildDefaultGraph());
    
    auto results = rec.recommendByGenres(lib, {"pop"}, nullptr);
    
    REQUIRE(!results.empty());
    CHECK(results.front().track->getMusicPath() == "3.mp3"); // A Pop Song
}

TEST_CASE("Recommender: ranking breaks ties with title", "[recommender]") {
    Library lib;
    // Both tracks have the exact same genre, so they get the same score
    lib.addTrack(Track({"Band A"}, "1.mp3", "Zebra", 200, {"rock"}));
    lib.addTrack(Track({"Band B"}, "2.mp3", "Apple", 200, {"rock"}));
    lib.addTrack(Track({"Band C"}, "3.mp3", "Seed", 200, {"rock"}));
    
    Recommender rec(Recommender::buildDefaultGraph());
    const Track* seed = lib.findByPath("3.mp3");
    
    auto results = rec.recommend(lib, *seed);
    REQUIRE(results.size() == 2);
    // Apple should come before Zebra
    CHECK(results[0].track->getTitle() == "Apple");
    CHECK(results[1].track->getTitle() == "Zebra");
}

TEST_CASE("Recommender: handles case-insensitive genres", "[recommender]") {
    Library lib;
    lib.addTrack(Track({"Band A"}, "1.mp3", "Song", 200, {"ROCK"}));
    
    Recommender rec(Recommender::buildDefaultGraph());
    auto results = rec.recommendByGenres(lib, {"rOcK"}, nullptr);
    
    REQUIRE(results.size() == 1);
    CHECK(results.front().score > 0.0f);
}
