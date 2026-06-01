#include <catch2/catch_test_macros.hpp>

#include "test_helpers.hpp"
#include "model/library.hpp"

using namespace test_helpers;

TEST_CASE("Library adds and stores tracks", "[library]") {
    Library lib;

    lib.addTrack(makeTrack("music/../music/track1.mp3", 100));
    lib.addTrack(makeTrack("music/track2.mp3", 200));

    REQUIRE(lib.getTracks().size() == 2);
}

TEST_CASE("Library finds tracks by normalized path", "[library]") {
    Library lib;

    lib.addTrack(makeTrack("music/../music/track1.mp3", 100));

    const Track* found = lib.findByPath("music/track1.mp3");
    REQUIRE(found != nullptr);
    CHECK(found->getDuration() == 100);

    const Track* normalized = lib.findByPath("music/../music/track1.mp3");
    REQUIRE(normalized != nullptr);
    CHECK(normalized->getDuration() == 100);
}

TEST_CASE("Library returns nullptr for missing path", "[library]") {
    Library lib;

    lib.addTrack(makeTrack("music/track1.mp3", 100));

    CHECK(lib.findByPath("missing.mp3") == nullptr);
}

TEST_CASE("Library ignores duplicate paths", "[library]") {
    Library lib;
    Track first = makeTrack("dup.mp3", 10);
    Track second = makeTrack("dup.mp3", 20);

    lib.addTrack(std::move(first));
    lib.addTrack(std::move(second));

    REQUIRE(lib.getTracks().size() == 1);
    const Track* found = lib.findByPath("dup.mp3");
    REQUIRE(found != nullptr);
    CHECK(found->getDuration() == 10);
}
