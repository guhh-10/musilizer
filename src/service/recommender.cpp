#include <algorithm>

#include "service/recommender.hpp"
#include "utils/string_utils.hpp"

// Recommender

Recommender::Recommender(GenreGraph graph) : graph_(std::move(graph)) {}

void Recommender::setGraph(GenreGraph graph) {
    graph_ = std::move(graph);
}

const GenreGraph& Recommender::graph() const {
    return graph_;
}

// Score candidate by summing edge weights for every (seedGenre → candidateGenre)
// pair. Comparison is case-insensitive.
float Recommender::scoreCandidate(
    const Track&                    candidate,
    const std::vector<std::string>& seedGenres) const
{
    float total = 0.0f;
    for (const std::string& sg : seedGenres) {
        const std::string sgL = utils::toLower(sg);
        for (const std::string& cg : candidate.getGenres()) {
            const std::string cgL = utils::toLower(cg);
            
            float w = graph_.weight(sgL, cgL);
            if (sgL == cgL && w == 0.0f) w = 1.0f;
            total += w;
        }
    }
    return total;
}

std::vector<RecommendResult> Recommender::recommend(
    const Library& lib,
    const Track&   seed,
    std::size_t    limit) const
{
    return recommendByGenres(lib, seed.getGenres(), &seed, limit);
}

std::vector<RecommendResult> Recommender::recommendByGenres(
    const Library&                  lib,
    const std::vector<std::string>& seedGenres,
    const Track*                    exclude,
    std::size_t                     limit) const
{
    std::vector<RecommendResult> scored;
    scored.reserve(lib.getTracks().size());

    for (const auto& [path, track] : lib.getTracks()) {
        if (exclude && track.getMusicPath() == exclude->getMusicPath())
            continue;
        float s = scoreCandidate(track, seedGenres);
        scored.push_back({&track, s});
    }

    // Check if anything scored above zero
    bool anyScored = std::any_of(scored.begin(), scored.end(),
        [](const RecommendResult& r) { return r.score > 0.0f; });

    if (anyScored) {
        // Remove zero-score entries
        scored.erase(std::remove_if(scored.begin(), scored.end(),
            [](const RecommendResult& r) { return r.score == 0.0f; }),
            scored.end());
    }
    // If nothing scored, return all (unranked fallback) – caller sees score 0.

    // Stable sort: higher score first; tie-break by title for determinism.
    std::stable_sort(scored.begin(), scored.end(),
        [](const RecommendResult& a, const RecommendResult& b) {
            if (a.score != b.score) return a.score > b.score;
            return a.track->getTitle() < b.track->getTitle();
        });

    if (limit > 0 && scored.size() > limit)
        scored.resize(limit);

    return scored;
}

// Default graph
//
// Edge semantics: weight(A → B) = how strongly "A listener will enjoy B".
//
// Weights in [0.0, 1.0]:
//   1.0  near-identical / subgenre
//   0.8  very strong crossover
//   0.6  strong crossover
//   0.4  moderate crossover
//   0.2  mild crossover
//   0.1  loose connection

GenreGraph Recommender::buildDefaultGraph() {
    GenreGraph g;

    // rock
    g.addEdge("rock",          "alternative",   0.8f);
    g.addEdge("rock",          "indie",         0.7f);
    g.addEdge("rock",          "metal",         0.6f);
    g.addEdge("rock",          "blues",         0.6f);
    g.addEdge("rock",          "pop",           0.4f);
    g.addEdge("rock",          "folk",          0.3f);
    g.addEdge("rock",          "punk",          0.6f);
    g.addEdge("rock",          "classic rock",  0.9f);

    g.addEdge("alternative",   "indie",         0.8f);
    g.addEdge("alternative",   "rock",          0.7f);
    g.addEdge("alternative",   "punk",          0.5f);
    g.addEdge("alternative",   "electronic",    0.4f);

    g.addEdge("indie",         "folk",          0.6f);
    g.addEdge("indie",         "alternative",   0.7f);
    g.addEdge("indie",         "pop",           0.5f);
    g.addEdge("indie",         "dream pop",     0.8f);

    g.addEdge("classic rock",  "rock",          0.9f);
    g.addEdge("classic rock",  "blues",         0.7f);
    g.addEdge("classic rock",  "hard rock",     0.8f);

    g.addEdge("punk",          "rock",          0.7f);
    g.addEdge("punk",          "alternative",   0.6f);
    g.addEdge("punk",          "hardcore",      0.8f);

    g.addEdge("hard rock",     "metal",         0.7f);
    g.addEdge("hard rock",     "rock",          0.8f);
    g.addEdge("hard rock",     "classic rock",  0.7f);

    // metal
    g.addEdge("metal",         "hard rock",     0.7f);
    g.addEdge("metal",         "rock",          0.5f);
    g.addEdge("metal",         "death metal",   0.8f);
    g.addEdge("metal",         "black metal",   0.7f);
    g.addEdge("metal",         "doom metal",    0.6f);
    g.addEdge("metal",         "prog",          0.5f);

    g.addEdge("death metal",   "metal",         0.9f);
    g.addEdge("death metal",   "black metal",   0.7f);
    g.addEdge("black metal",   "metal",         0.9f);
    g.addEdge("black metal",   "doom metal",    0.6f);
    g.addEdge("doom metal",    "metal",         0.8f);
    g.addEdge("doom metal",    "ambient",       0.3f);

    // pop
    g.addEdge("pop",           "r&b",           0.6f);
    g.addEdge("pop",           "dance",         0.6f);
    g.addEdge("pop",           "indie",         0.4f);
    g.addEdge("pop",           "electronic",    0.4f);
    g.addEdge("pop",           "folk",          0.3f);
    g.addEdge("pop",           "hip-hop",       0.4f);

    g.addEdge("dream pop",     "shoegaze",      0.8f);
    g.addEdge("dream pop",     "indie",         0.7f);
    g.addEdge("dream pop",     "ambient",       0.5f);
    g.addEdge("dream pop",     "pop",           0.5f);

    // electronic
    g.addEdge("electronic",    "ambient",       0.7f);
    g.addEdge("electronic",    "dance",         0.7f);
    g.addEdge("electronic",    "techno",        0.8f);
    g.addEdge("electronic",    "house",         0.8f);
    g.addEdge("electronic",    "idm",           0.7f);
    g.addEdge("electronic",    "synthpop",      0.7f);
    g.addEdge("electronic",    "trip-hop",      0.5f);

    g.addEdge("ambient",       "electronic",    0.6f);
    g.addEdge("ambient",       "new age",       0.6f);
    g.addEdge("ambient",       "dream pop",     0.4f);
    g.addEdge("ambient",       "classical",     0.3f);

    g.addEdge("techno",        "electronic",    0.9f);
    g.addEdge("techno",        "house",         0.7f);
    g.addEdge("techno",        "industrial",    0.5f);

    g.addEdge("house",         "electronic",    0.9f);
    g.addEdge("house",         "techno",        0.7f);
    g.addEdge("house",         "dance",         0.8f);
    g.addEdge("house",         "r&b",           0.3f);

    g.addEdge("synthpop",      "electronic",    0.8f);
    g.addEdge("synthpop",      "pop",           0.6f);
    g.addEdge("synthpop",      "new wave",      0.8f);

    g.addEdge("idm",           "electronic",    0.9f);
    g.addEdge("idm",           "ambient",       0.6f);
    g.addEdge("idm",           "glitch",        0.8f);

    g.addEdge("trip-hop",      "electronic",    0.7f);
    g.addEdge("trip-hop",      "hip-hop",       0.6f);
    g.addEdge("trip-hop",      "jazz",          0.5f);
    g.addEdge("trip-hop",      "r&b",           0.5f);

    g.addEdge("dance",         "electronic",    0.8f);
    g.addEdge("dance",         "pop",           0.6f);
    g.addEdge("dance",         "house",         0.7f);

    // hip-hop
    g.addEdge("hip-hop",       "r&b",           0.7f);
    g.addEdge("hip-hop",       "trap",          0.8f);
    g.addEdge("hip-hop",       "lo-fi",         0.6f);
    g.addEdge("hip-hop",       "trip-hop",      0.5f);
    g.addEdge("hip-hop",       "funk",          0.5f);
    g.addEdge("hip-hop",       "jazz",          0.4f);

    g.addEdge("trap",          "hip-hop",       0.9f);
    g.addEdge("trap",          "r&b",           0.6f);
    g.addEdge("trap",          "electronic",    0.4f);

    g.addEdge("lo-fi",         "hip-hop",       0.7f);
    g.addEdge("lo-fi",         "jazz",          0.5f);
    g.addEdge("lo-fi",         "ambient",       0.5f);
    g.addEdge("lo-fi",         "chillout",      0.7f);

    // r&b / soul / funk
    g.addEdge("r&b",           "soul",          0.9f);
    g.addEdge("r&b",           "hip-hop",       0.6f);
    g.addEdge("r&b",           "funk",          0.7f);
    g.addEdge("r&b",           "pop",           0.5f);
    g.addEdge("r&b",           "jazz",          0.4f);

    g.addEdge("soul",          "r&b",           0.9f);
    g.addEdge("soul",          "funk",          0.7f);
    g.addEdge("soul",          "gospel",        0.7f);
    g.addEdge("soul",          "jazz",          0.5f);

    g.addEdge("funk",          "soul",          0.8f);
    g.addEdge("funk",          "r&b",           0.7f);
    g.addEdge("funk",          "jazz",          0.5f);
    g.addEdge("funk",          "disco",         0.7f);

    g.addEdge("disco",         "funk",          0.7f);
    g.addEdge("disco",         "dance",         0.7f);
    g.addEdge("disco",         "house",         0.5f);

    // jazz
    g.addEdge("jazz",          "blues",         0.7f);
    g.addEdge("jazz",          "soul",          0.6f);
    g.addEdge("jazz",          "bossa nova",    0.7f);
    g.addEdge("jazz",          "classical",     0.4f);
    g.addEdge("jazz",          "fusion",        0.8f);
    g.addEdge("jazz",          "funk",          0.5f);
    g.addEdge("jazz",          "lo-fi",         0.4f);

    g.addEdge("fusion",        "jazz",          0.9f);
    g.addEdge("fusion",        "rock",          0.5f);
    g.addEdge("fusion",        "electronic",    0.4f);

    g.addEdge("bossa nova",    "jazz",          0.8f);
    g.addEdge("bossa nova",    "latin",         0.7f);
    g.addEdge("bossa nova",    "samba",         0.6f);

    // blues
    g.addEdge("blues",         "jazz",          0.7f);
    g.addEdge("blues",         "rock",          0.6f);
    g.addEdge("blues",         "soul",          0.6f);
    g.addEdge("blues",         "country",       0.4f);

    // classical
    g.addEdge("classical",     "orchestral",    0.9f);
    g.addEdge("classical",     "opera",         0.7f);
    g.addEdge("classical",     "chamber",       0.8f);
    g.addEdge("classical",     "ambient",       0.3f);
    g.addEdge("classical",     "new age",       0.3f);
    g.addEdge("classical",     "jazz",          0.3f);

    g.addEdge("orchestral",    "classical",     0.9f);
    g.addEdge("orchestral",    "soundtrack",    0.7f);
    g.addEdge("orchestral",    "ambient",       0.4f);

    g.addEdge("soundtrack",    "orchestral",    0.7f);
    g.addEdge("soundtrack",    "ambient",       0.5f);
    g.addEdge("soundtrack",    "classical",     0.4f);

    // folk / country
    g.addEdge("folk",          "country",       0.6f);
    g.addEdge("folk",          "indie",         0.6f);
    g.addEdge("folk",          "singer-songwriter", 0.8f);
    g.addEdge("folk",          "bluegrass",     0.7f);
    g.addEdge("folk",          "acoustic",      0.8f);

    g.addEdge("country",       "folk",          0.6f);
    g.addEdge("country",       "bluegrass",     0.7f);
    g.addEdge("country",       "singer-songwriter", 0.6f);
    g.addEdge("country",       "americana",     0.8f);

    g.addEdge("bluegrass",     "country",       0.8f);
    g.addEdge("bluegrass",     "folk",          0.7f);

    g.addEdge("singer-songwriter", "folk",      0.7f);
    g.addEdge("singer-songwriter", "indie",     0.6f);
    g.addEdge("singer-songwriter", "acoustic",  0.8f);

    g.addEdge("americana",     "country",       0.8f);
    g.addEdge("americana",     "folk",          0.7f);
    g.addEdge("americana",     "blues",         0.5f);

    // reggae / latin
    g.addEdge("reggae",        "dancehall",     0.8f);
    g.addEdge("reggae",        "dub",           0.8f);
    g.addEdge("reggae",        "ska",           0.7f);
    g.addEdge("reggae",        "world",         0.4f);

    g.addEdge("latin",         "bossa nova",    0.7f);
    g.addEdge("latin",         "salsa",         0.8f);
    g.addEdge("latin",         "samba",         0.7f);
    g.addEdge("latin",         "reggaeton",     0.6f);

    // shoegaze / post-rock
    g.addEdge("shoegaze",      "dream pop",     0.8f);
    g.addEdge("shoegaze",      "alternative",   0.7f);
    g.addEdge("shoegaze",      "noise rock",    0.6f);
    g.addEdge("shoegaze",      "post-rock",     0.7f);

    g.addEdge("post-rock",     "shoegaze",      0.6f);
    g.addEdge("post-rock",     "ambient",       0.6f);
    g.addEdge("post-rock",     "progressive",   0.6f);
    g.addEdge("post-rock",     "math rock",     0.7f);

    g.addEdge("prog",          "progressive",   1.0f); // alias
    g.addEdge("progressive",   "prog",          1.0f);
    g.addEdge("progressive",   "rock",          0.6f);
    g.addEdge("progressive",   "metal",         0.5f);
    g.addEdge("progressive",   "jazz",          0.4f);
    g.addEdge("progressive",   "post-rock",     0.6f);

    // misc
    g.addEdge("new wave",      "synthpop",      0.8f);
    g.addEdge("new wave",      "post-punk",     0.8f);
    g.addEdge("new wave",      "alternative",   0.6f);

    g.addEdge("post-punk",     "punk",          0.7f);
    g.addEdge("post-punk",     "new wave",      0.7f);
    g.addEdge("post-punk",     "gothic",        0.7f);
    g.addEdge("post-punk",     "alternative",   0.6f);

    g.addEdge("gothic",        "post-punk",     0.7f);
    g.addEdge("gothic",        "industrial",    0.6f);
    g.addEdge("gothic",        "doom metal",    0.5f);

    g.addEdge("industrial",    "electronic",    0.6f);
    g.addEdge("industrial",    "metal",         0.5f);
    g.addEdge("industrial",    "noise",         0.7f);

    g.addEdge("chillout",      "ambient",       0.7f);
    g.addEdge("chillout",      "lo-fi",         0.7f);
    g.addEdge("chillout",      "electronic",    0.5f);

    g.addEdge("gospel",        "soul",          0.8f);
    g.addEdge("gospel",        "r&b",           0.6f);

    g.addEdge("world",         "folk",          0.5f);
    g.addEdge("world",         "latin",         0.5f);
    g.addEdge("world",         "reggae",        0.4f);

    return g;
}