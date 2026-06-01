#include <catch2/catch_test_macros.hpp>

#include "test_helpers.hpp"
#include "model/play_history.hpp"

using namespace test_helpers;

TEST_CASE("PlayHistory navigation", "[play_history]") {
    PlayHistory history;
    Track t1 = makeTrack("a.mp3");
    Track t2 = makeTrack("b.mp3");
    Track t3 = makeTrack("c.mp3");

    CHECK_FALSE(history.current().has_value());
    CHECK_FALSE(history.back().has_value());
    CHECK_FALSE(history.canGoForward());

    history.push(t1);
    history.push(t2);
    history.push(t3);

    REQUIRE(history.current().has_value());
    CHECK(history.current().value() == fs::path("c.mp3"));
    CHECK(history.canGoBack());

    auto back = history.back();
    REQUIRE(back.has_value());
    CHECK(back.value() == fs::path("b.mp3"));

    auto forward = history.forward();
    REQUIRE(forward.has_value());
    CHECK(forward.value() == fs::path("c.mp3"));
}

TEST_CASE("PlayHistory clear resets state", "[play_history]") {
    PlayHistory history;
    history.push(makeTrack("a.mp3"));

    history.clear();
    CHECK_FALSE(history.current().has_value());
}

TEST_CASE("PlayHistory branching clears forward", "[play_history]") {
    PlayHistory history;
    Track t1 = makeTrack("a.mp3");
    Track t2 = makeTrack("b.mp3");
    Track t3 = makeTrack("c.mp3");
    Track t4 = makeTrack("d.mp3");

    history.push(t1);
    history.push(t2);
    history.push(t3);

    auto back = history.back();
    REQUIRE(back.has_value());
    CHECK(back.value() == fs::path("b.mp3"));

    history.push(t4);
    CHECK_FALSE(history.canGoForward());
    CHECK(history.current().value() == fs::path("d.mp3"));
    CHECK_FALSE(history.forward().has_value());
}

TEST_CASE("PlayHistory underflow stays at first entry", "[play_history]") {
    PlayHistory underflow;
    Track t1 = makeTrack("a.mp3");
    Track t2 = makeTrack("b.mp3");

    underflow.push(t1);
    underflow.push(t2);
    auto back1 = underflow.back();
    REQUIRE(back1.has_value());
    CHECK(back1.value() == fs::path("a.mp3"));
    auto back2 = underflow.back();
    CHECK_FALSE(back2.has_value());
    CHECK(underflow.current().value() == fs::path("a.mp3"));
}

TEST_CASE("PlayHistory max size", "[play_history]") {
    PlayHistory history;
    for (int i = 0; i < config::MAX_HISTORY; ++i) {
        history.push(makeTrack(std::to_string(i) + ".mp3"));
    }

    auto back = history.back();
    REQUIRE(back.has_value());
    CHECK(back.value() == fs::path(std::to_string(config::MAX_HISTORY - 2) + ".mp3"));

    history.push(makeTrack("new.mp3"));

    CHECK(history.getHistory().size() == static_cast<size_t>(config::MAX_HISTORY));
    REQUIRE(history.current().has_value());
    CHECK(history.current().value() == fs::path("new.mp3"));
    CHECK_FALSE(history.canGoForward());
}
