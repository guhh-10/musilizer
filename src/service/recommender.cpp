#include <algorithm>

#include "service/recommender.hpp"
#include "service/default_genre_graph.hpp"
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

GenreGraph Recommender::buildDefaultGraph() {
    return buildDefaultGenreGraph();
}