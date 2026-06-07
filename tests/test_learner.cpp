#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "service/genre_graph.hpp"
#include "service/genre_graph_learner.hpp"
#include "service/recommender.hpp"
#include "model/track.hpp"
#include "model/library.hpp"

// ── helpers ───────────────────────────────────────────────────────────────────

namespace {

// A tiny prior: rock to metal=0.6, rock to jazz=0.2
GenreGraph tinyPrior() {
    GenreGraph g;
    g.addEdge("rock", "metal", 0.6f);
    g.addEdge("rock", "jazz",  0.2f);
    return g;
}

Track makeTrack(const std::string& path,
                std::vector<std::string> genres) {
    return Track({"A"}, path, "T", 200, std::move(genres));
}

} // namespace

// ── weight formula ────────────────────────────────────────────────────────────

TEST_CASE("Learner: prior alone produces expected weights", "[learner]") {
    // prior(rock to metal) = 0.6 * 10 = 6
    // prior(rock to jazz)  = 0.2 * 10 = 2
    // prior_total(rock) = 8
    // weight(rock to metal) = 6/8 = 0.75
    // weight(rock to jazz)  = 2/8 = 0.25
    GenreGraphLearner learner(tinyPrior());
    GenreGraph g = learner.toGraph();

    CHECK(g.weight("rock", "metal") == Catch::Approx(0.75f).epsilon(0.001f));
    CHECK(g.weight("rock", "jazz")  == Catch::Approx(0.25f).epsilon(0.001f));
}

TEST_CASE("Learner: single completion shifts weight toward observed edge", "[learner]") {
    GenreGraphLearner learner(tinyPrior());

    // Observe one rock to metal completion.
    // observed(rock to metal) = 1, observed_total(rock) = 1
    // weight(rock to metal) = (6+1)/(8+1) = 7/9 ≈ 0.778
    // weight(rock to jazz)  = (2+0)/(8+1) = 2/9 ≈ 0.222
    learner.observeCompletion({"rock"}, {"metal"});
    GenreGraph g = learner.toGraph();

    CHECK(g.weight("rock", "metal") == Catch::Approx(7.0f / 9.0f).epsilon(0.001f));
    CHECK(g.weight("rock", "jazz")  == Catch::Approx(2.0f / 9.0f).epsilon(0.001f));
}

TEST_CASE("Learner: many completions eventually dominate the prior", "[learner]") {
    GenreGraphLearner learner(tinyPrior());

    // After 100 rock to jazz completions the observed signal should dominate.
    // weight(rock to jazz) ≈ (2+100)/(8+100) = 102/108 ≈ 0.944
    for (int i = 0; i < 100; ++i)
        learner.observeCompletion({"rock"}, {"jazz"});

    GenreGraph g = learner.toGraph();
    CHECK(g.weight("rock", "jazz") == Catch::Approx(102.0f / 108.0f).epsilon(0.001f));
    // metal weight should now be small
    CHECK(g.weight("rock", "metal") < 0.2f);
}

TEST_CASE("Learner: skip decrements observed count", "[learner]") {
    GenreGraphLearner learner(tinyPrior());

    // Two completions then one skip on rock to metal.
    learner.observeCompletion({"rock"}, {"metal"});
    learner.observeCompletion({"rock"}, {"metal"});
    learner.observeSkip({"rock"}, {"metal"});

    // observed(rock to metal) = max(0, 2 - 0.5) = 1.5
    // observed_total(rock) = 3 events  to  total observed delta = 2+2-0.5 = 3.5
    // But rowSum sums cell values, not event counts.
    // observed cells: metal=1.5 (floor applied per cell)
    // observed_total = 1.5
    // weight = (6+1.5)/(8+1.5) = 7.5/9.5
    GenreGraph g = learner.toGraph();
    CHECK(g.weight("rock", "metal") == Catch::Approx(7.5f / 9.5f).epsilon(0.001f));
}

TEST_CASE("Learner: observed count is floored at 0 per cell", "[learner]") {
    GenreGraphLearner learner(tinyPrior());

    // Skip an edge that has zero observed count — should not go negative.
    learner.observeSkip({"rock"}, {"jazz"});
    learner.observeSkip({"rock"}, {"jazz"});
    learner.observeSkip({"rock"}, {"jazz"});

    GenreGraph g = learner.toGraph();
    // observed(rock to jazz) floored at 0, so weight = prior only = 2/8
    CHECK(g.weight("rock", "jazz") == Catch::Approx(0.25f).epsilon(0.001f));
}

// ── new edge discovery ────────────────────────────────────────────────────────

TEST_CASE("Learner: edge not in prior emerges from observations", "[learner]") {
    GenreGraphLearner learner(tinyPrior());

    // rock to classical is not in the prior.
    // After enough observations it should appear.
    for (int i = 0; i < 5; ++i)
        learner.observeCompletion({"rock"}, {"classical"});

    GenreGraph g = learner.toGraph();
    // observed(rock to classical) = 5, prior = 0
    // prior_total = 8, observed_total = 5
    // weight = 5/13 ≈ 0.385
    CHECK(g.weight("rock", "classical") == Catch::Approx(5.0f / 13.0f).epsilon(0.001f));
}

TEST_CASE("Learner: multi-genre observation updates all pairs", "[learner]") {
    GenreGraphLearner learner(tinyPrior());

    // prev has two genres, next has two genres  to  4 pairs updated.
    learner.observeCompletion({"rock", "metal"}, {"jazz", "blues"});

    GenreGraph g = learner.toGraph();
    CHECK(g.weight("rock",  "jazz")  > 0.0f);
    CHECK(g.weight("rock",  "blues") > 0.0f);
    CHECK(g.weight("metal", "jazz")  > 0.0f);
    CHECK(g.weight("metal", "blues") > 0.0f);
}

// ── totalObservations ─────────────────────────────────────────────────────────

TEST_CASE("Learner: totalObservations counts all observe calls", "[learner]") {
    GenreGraphLearner learner(tinyPrior());
    CHECK(learner.totalObservations() == 0);

    learner.observeCompletion({"rock"}, {"metal"});
    CHECK(learner.totalObservations() == 1);

    learner.observeSkip({"rock"}, {"jazz"});
    CHECK(learner.totalObservations() == 2);
}

// ── persistence round-trip ────────────────────────────────────────────────────

TEST_CASE("Learner: setObservedCounts round-trips counts", "[learner]") {
    GenreGraphLearner learner(tinyPrior());

    for (int i = 0; i < 3; ++i)
        learner.observeCompletion({"rock"}, {"metal"});

    // Simulate save + load by extracting and re-injecting counts.
    GenreGraphLearner learner2(tinyPrior());
    learner2.setObservedCounts(learner.observedCounts());

    GenreGraph g1 = learner.toGraph();
    GenreGraph g2 = learner2.toGraph();

    CHECK(g1.weight("rock", "metal") == Catch::Approx(g2.weight("rock", "metal")).epsilon(0.001f));
    CHECK(g1.weight("rock", "jazz")  == Catch::Approx(g2.weight("rock", "jazz")) .epsilon(0.001f));
}

// ── integration with Recommender ─────────────────────────────────────────────

TEST_CASE("Learner+Recommender: repeated jazz completions surface jazz tracks", "[learner]") {
    Library lib;
    lib.addTrack(makeTrack("metal.mp3",  {"metal"}));
    lib.addTrack(makeTrack("jazz.mp3",   {"jazz"}));
    lib.addTrack(makeTrack("blues.mp3",  {"blues"}));

    // Start from default prior — rock prefers metal over jazz.
    GenreGraphLearner learner(Recommender::buildDefaultGraph());
    Recommender rec(learner.toGraph());

    Track seed = makeTrack("seed.mp3", {"rock"});

    // Before learning: metal should rank above jazz (rock to metal > rock to jazz in prior).
    {
        auto results = rec.recommendByGenres(lib, seed.getGenres(), nullptr, 10);
        auto metalIt = std::find_if(results.begin(), results.end(),
            [](const RecommendResult& r) { return r.track->getMusicPath() == "metal.mp3"; });
        auto jazzIt  = std::find_if(results.begin(), results.end(),
            [](const RecommendResult& r) { return r.track->getMusicPath() == "jazz.mp3"; });
        if (metalIt != results.end() && jazzIt != results.end())
            CHECK(metalIt->score >= jazzIt->score);
    }

    // Observe 20 rock to jazz completions.
    for (int i = 0; i < 20; ++i)
        learner.observeCompletion({"rock"}, {"jazz"});
    rec.setGraph(learner.toGraph());

    // After learning: jazz should rank above metal.
    {
        auto results = rec.recommendByGenres(lib, seed.getGenres(), nullptr, 10);
        auto metalIt = std::find_if(results.begin(), results.end(),
            [](const RecommendResult& r) { return r.track->getMusicPath() == "metal.mp3"; });
        auto jazzIt  = std::find_if(results.begin(), results.end(),
            [](const RecommendResult& r) { return r.track->getMusicPath() == "jazz.mp3"; });
        REQUIRE(jazzIt  != results.end());
        REQUIRE(metalIt != results.end());
        CHECK(jazzIt->score > metalIt->score);
    }
}

TEST_CASE("Learner+Recommender: skips suppress an edge over time", "[learner]") {
    Library lib;
    lib.addTrack(makeTrack("metal.mp3", {"metal"}));
    lib.addTrack(makeTrack("jazz.mp3",  {"jazz"}));

    GenreGraphLearner learner(tinyPrior());
    Recommender rec(learner.toGraph());

    Track seed = makeTrack("seed.mp3", {"rock"});

    // Initially metal scores higher than jazz (prior 0.75 vs 0.25).
    float metalBefore = rec.recommendByGenres(lib, seed.getGenres(), nullptr, 10).front().score;
    (void)metalBefore;

    // Skips alone lower metal's observed count (floored at 0) but cannot
    // invert the prior — the prior pseudo-counts are unchanged.  To flip the
    // ranking we must also reinforce jazz with completions.
    for (int i = 0; i < 30; ++i)
        learner.observeSkip({"rock"}, {"metal"});
    for (int i = 0; i < 30; ++i)
        learner.observeCompletion({"rock"}, {"jazz"});
    rec.setGraph(learner.toGraph());

    auto results = rec.recommendByGenres(lib, seed.getGenres(), nullptr, 10);
    auto metalIt = std::find_if(results.begin(), results.end(),
        [](const RecommendResult& r) { return r.track->getMusicPath() == "metal.mp3"; });
    auto jazzIt  = std::find_if(results.begin(), results.end(),
        [](const RecommendResult& r) { return r.track->getMusicPath() == "jazz.mp3"; });

    REQUIRE(metalIt != results.end());
    REQUIRE(jazzIt  != results.end());
    // Skips suppressed metal; jazz completions reinforced jazz — jazz wins.
    CHECK(jazzIt->score > metalIt->score);
    // Metal weight must also be strictly lower than its initial prior weight.
    CHECK(metalIt->score < metalBefore);
}