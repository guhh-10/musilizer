#pragma once
#include <vector>
#include <string>

#include "model/library.hpp"
#include "model/track.hpp"
#include "service/genre_graph.hpp"

struct RecommendResult {
    const Track* track;
    float        score;   // cumulative genre, higher = better match
};

class Recommender {
    private:
        GenreGraph graph_;

        // Score one candidate track against a set of seed genres.
        float scoreCandidate(
            const Track&                    candidate,
            const std::vector<std::string>& seedGenres) const;

    public:
        // Build with a pre-populated graph.
        explicit Recommender(GenreGraph graph);

        // Replace the graph (e.g. after user feedback updates weights).
        void setGraph(GenreGraph graph);

        const GenreGraph& graph() const;

        // Return up to `limit` tracks from `lib` that are most similar to `seed`.
        // Tracks with score == 0.0f are omitted unless all candidates score 0.
        std::vector<RecommendResult> recommend(
            const Library&          lib,
            const Track&            seed,
            std::size_t             limit = 10) const;

        // Convenience: recommend based on an explicit set of seed genres
        // (useful when you want cross-playlist recommendations without a
        // single anchor track).
        std::vector<RecommendResult> recommendByGenres(
            const Library&                  lib,
            const std::vector<std::string>& seedGenres,
            const Track*                    exclude,       // may be nullptr
            std::size_t                     limit = 10) const;

        // Factory: returns a GenreGraph seeded with reasonable cross-genre weights.
        static GenreGraph buildDefaultGraph();
};