#include <catch2/catch_test_macros.hpp>

#include "test_helpers.hpp"
#include "model/queue.hpp"

using namespace test_helpers;

TEST_CASE("Queue navigates forward", "[queue]") {
    Track t1 = makeTrack("a.mp3");
    Track t2 = makeTrack("b.mp3");
    Track t3 = makeTrack("c.mp3");

    Queue q;
    q.load({&t1, &t2, &t3});

    REQUIRE(q.current().has_value());
    CHECK(q.current().value() == fs::path("a.mp3"));
    CHECK(q.hasNext());

    auto next1 = q.next();
    REQUIRE(next1.has_value());
    CHECK(next1.value() == fs::path("b.mp3"));

    auto next2 = q.next();
    REQUIRE(next2.has_value());
    CHECK(next2.value() == fs::path("c.mp3"));

    CHECK_FALSE(q.hasNext());
    CHECK_FALSE(q.next().has_value());
}

TEST_CASE("Queue repeat reloads original order", "[queue]") {
    Track t1 = makeTrack("a.mp3");
    Track t2 = makeTrack("b.mp3");

    Queue q;
    q.load({&t1, &t2});
    q.setRepeat(true);
    q.next();
    auto repeatNext = q.next();
    REQUIRE(repeatNext.has_value());
    // repeat reloads original_order and returns the front without popping
    CHECK(repeatNext.value() == fs::path("a.mp3"));
    q.setRepeat(false);
}

TEST_CASE("Queue shuffle toggle preserves current", "[queue]") {
    Track t1 = makeTrack("a.mp3");
    Track t2 = makeTrack("b.mp3");
    Track t3 = makeTrack("c.mp3");

    Queue q;
    q.load({&t1, &t2, &t3});
    q.setShuffle(true);
    auto current = q.current();
    REQUIRE(current.has_value());
    CHECK(current.value() == fs::path("a.mp3"));

    q.setShuffle(false);
    auto currentAfter = q.current();
    REQUIRE(currentAfter.has_value());
    CHECK(currentAfter.value() == fs::path("a.mp3"));
}

TEST_CASE("Queue allows shuffle flag on empty queue", "[queue]") {
    Queue empty;
    empty.setShuffle(true);
    CHECK(empty.isShuffle());
}

TEST_CASE("Queue empty load has no current", "[queue]") {
    Queue empty;
    empty.load({});
    CHECK_FALSE(empty.current().has_value());
    CHECK_FALSE(empty.hasNext());
}

TEST_CASE("Queue repeat on single track keeps current", "[queue]") {
    Track t1 = makeTrack("a.mp3");

    Queue single;
    single.load({&t1});
    single.setRepeat(true);
    auto repeatNext = single.next();
    REQUIRE(repeatNext.has_value());
    CHECK(repeatNext.value() == fs::path("a.mp3"));
    CHECK(single.current().value() == fs::path("a.mp3"));
    single.setRepeat(false);
}

TEST_CASE("Queue add-to-front respects repeat", "[queue]") {
    Track t1 = makeTrack("a.mp3");

    Queue frontEmpty;
    frontEmpty.addTrackToFront(t1);
    frontEmpty.setRepeat(true);
    auto frontRepeat = frontEmpty.next();
    REQUIRE(frontRepeat.has_value());
    CHECK(frontRepeat.value() == fs::path("a.mp3"));
    CHECK(frontEmpty.current().value() == fs::path("a.mp3"));
}

TEST_CASE("Queue shuffle restore keeps ordering", "[queue]") {
    Track t1 = makeTrack("a.mp3");
    Track t2 = makeTrack("b.mp3");
    Track t3 = makeTrack("c.mp3");

    Queue shuffleRestore;
    shuffleRestore.load({&t1, &t2, &t3});
    shuffleRestore.setShuffle(true);
    auto advanced = shuffleRestore.next();
    REQUIRE(advanced.has_value());
    fs::path currentBefore = advanced.value();
    shuffleRestore.setShuffle(false);
    auto currentAfter = shuffleRestore.current();
    REQUIRE(currentAfter.has_value());
    CHECK(currentAfter.value() == currentBefore);

    auto expectedNext = [&](const fs::path& current) {
        if (current == fs::path("a.mp3"))
            return fs::path("b.mp3");
        if (current == fs::path("b.mp3"))
            return fs::path("c.mp3");
        return fs::path("a.mp3");
    };

    auto nextAfterRestore = shuffleRestore.next();
    REQUIRE(nextAfterRestore.has_value());
    CHECK(nextAfterRestore.value() == expectedNext(currentBefore));
}

TEST_CASE("Queue add-to-front/back behavior", "[queue]") {
    Track t1 = makeTrack("a.mp3");
    Track t2 = makeTrack("b.mp3");
    Track t3 = makeTrack("c.mp3");

    Queue q;
    q.addTrackToFront(t1);
    REQUIRE(q.current().has_value());
    CHECK(q.current().value() == fs::path("a.mp3"));

    q.addTrackToFront(t2);
    auto next = q.next();
    REQUIRE(next.has_value());
    // addTrackToFront inserts as "play next" when queue is non-empty
    CHECK(next.value() == fs::path("b.mp3"));

    q.addTrackToBack(t3);
    next = q.next();
    REQUIRE(next.has_value());
    CHECK(next.value() == fs::path("c.mp3"));
}
