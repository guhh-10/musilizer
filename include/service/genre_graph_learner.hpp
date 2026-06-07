#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "service/genre_graph.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// GenreGraphLearner
//
// Implements a Bayesian prior + observed co-play hybrid.
//
// Weight formula for edge A → B:
//
//   weight(A→B) = ( prior(A→B)  +  observed(A→B) )
//               / ( prior_total(A)  +  observed_total(A) )
//
// Where:
//   prior(A→B)       = default_graph.weight(A→B) * PRIOR_STRENGTH
//   prior_total(A)   = sum of all prior(A→*) values
//   observed(A→B)    = accumulated co-play count (completions add +1, skips add
//                      -SKIP_PENALTY, both floored at 0 per cell)
//   observed_total(A)= sum of all observed(A→*) values
//
// PRIOR_STRENGTH controls how many real observations are needed before the
// prior is overridden.  At PRIOR_STRENGTH = 10, an edge with a default weight
// of 0.6 has 6 pseudo-counts; 6 real completions are needed to shift it
// appreciably.
//
// The learner stores only the *raw counts*, not the computed weights.  Counts
// are lossless and allow recalculating weights at any time without losing
// information.  Call toGraph() to materialise a GenreGraph from current state.
// ─────────────────────────────────────────────────────────────────────────────

class GenreGraphLearner {
    public:
        static constexpr float PRIOR_STRENGTH = 10.0f;
        static constexpr float SKIP_PENALTY   =  0.5f;  // subtracted on skip

        // Initialise with the default prior graph.
        // All edges from `prior` are converted to pseudo-counts.
        explicit GenreGraphLearner(const GenreGraph& prior);


        // Record a natural completion transition: prev finished, next started.
        // Increments coplay[gA][gB] by +1 for every genre pair (gA, gB)
        // where gA ∈ prev.genres and gB ∈ next.genres.
        void observeCompletion(const std::vector<std::string>& prevGenres,
                            const std::vector<std::string>& nextGenres);

        // Record a skip transition: user skipped prev before it finished.
        // Decrements coplay[gA][gB] by SKIP_PENALTY, floored at 0.
        void observeSkip(const std::vector<std::string>& prevGenres,
                        const std::vector<std::string>& nextGenres);


        // Recompute and return a GenreGraph with weights derived from the
        // current prior + observed counts.  Edges with weight < MIN_WEIGHT
        // are omitted to keep the graph sparse.
        GenreGraph toGraph(float minWeight = 0.01f) const;


        // Raw count table, keyed [from][to].  Used by Persistence to serialise.
        using CountTable = std::unordered_map<std::string,
                            std::unordered_map<std::string, float>>;

        const CountTable& observedCounts() const;
        const CountTable& priorCounts()    const;

        // Replace observed counts (called by Persistence::loadLearner).
        // The prior is not replaced — it is always re-derived from the GenreGraph
        // passed to the constructor.
        void setObservedCounts(CountTable counts);

        // Total observed events (completions + skips) so callers can tell how
        // mature the model is.
        int totalObservations() const;

    private:
        CountTable prior_;     // pseudo-counts from buildDefaultGraph()
        CountTable observed_;  // real play data
        int        totalObs_ = 0;

        void addObservation(const std::vector<std::string>& prevGenres,
                            const std::vector<std::string>& nextGenres,
                            float delta);

        // Row sum of a CountTable row.  Returns 0 if row absent.
        static float rowSum(const CountTable& table, const std::string& from);
    
};