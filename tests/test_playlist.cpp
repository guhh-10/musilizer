#include <catch2/catch_test_macros.hpp>

#include "test_helpers.hpp"
#include "model/library.hpp"
#include "model/playlist.hpp"

using namespace test_helpers;

TEST_CASE("Playlist adds tracks and reports size", "[playlist]") {
    Playlist pl("Mix");
    CHECK(pl.isEmpty());

    pl.addTrack(makeTrack("a.mp3", 90));
    pl.addTrack(makeTrack("b.mp3", 110));
    pl.addTrack(makeTrack("c.mp3", 50));

    CHECK(pl.size() == 3);
    CHECK(pl.getTrack(0) == fs::path("a.mp3"));
}

TEST_CASE("Playlist getTrack throws out-of-range", "[playlist]") {
    Playlist pl("Mix");
    pl.addTrack(makeTrack("a.mp3", 90));

    CHECK_THROWS_AS(pl.getTrack(99), std::out_of_range);
}

TEST_CASE("Playlist removeTrack reduces size", "[playlist]") {
    Playlist pl("Mix");
    pl.addTrack(makeTrack("a.mp3", 90));
    pl.addTrack(makeTrack("b.mp3", 110));

    pl.removeTrack("b.mp3");
    CHECK(pl.size() == 1);
}

TEST_CASE("Playlist moveTrack handles bounds", "[playlist]") {
    Playlist pl("Mix");
    pl.addTrack(makeTrack("a.mp3", 90));
    pl.addTrack(makeTrack("b.mp3", 110));

    auto before = pl.getTrack(0);
    pl.moveTrack(0, 0);
    CHECK(pl.getTrack(0) == before);

    pl.moveTrack(0, 1);
    CHECK(pl.getTrack(1) == fs::path("a.mp3"));

    pl.moveTrack(-1, 2);
    pl.moveTrack(0, 99);
    CHECK(pl.size() == 2);
}

TEST_CASE("Playlist resolve uses library and duration", "[playlist]") {
    Library lib;
    Track t1 = makeTrack("a.mp3", 90);
    Track t2 = makeTrack("b.mp3", 110);
    lib.addTrack(t1);
    lib.addTrack(t2);

    Playlist pl("Mix");
    pl.addTrack(t1);
    pl.addTrack(t2);
    pl.addTrack(makeTrack("c.mp3", 50)); // not in library

    // c.mp3 is not in the library, so only library tracks contribute
    CHECK(pl.getTotalDuration(lib) == 200);
    auto resolved = pl.resolve(lib);
    CHECK(resolved.size() == 2);
    CHECK(resolved.front()->getMusicPath() == fs::path("a.mp3"));
}

TEST_CASE("Playlist clear resets state", "[playlist]") {
    Playlist pl("Mix");
    pl.addTrack(makeTrack("a.mp3", 90));

    pl.clear();
    CHECK(pl.isEmpty());
}

TEST_CASE("Playlist resolve with empty library", "[playlist]") {
    Playlist pl("Empty");
    pl.addTrack(makeTrack("a.mp3", 90));

    Library empty;
    auto resolved = pl.resolve(empty);
    CHECK(resolved.empty());
}
