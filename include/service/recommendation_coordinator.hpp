#pragma once

#include "service/recommender.hpp"
#include "service/genre_graph_learner.hpp"
#include "model/track.hpp"
#include "model/library.hpp"
#include <functional>
#include <vector>

class RecommendationCoordinator {
private:
    GenreGraphLearner learner_;
    Recommender recommender_;

public:
    std::function<void()> onRecommendationsReady;

    RecommendationCoordinator();

    void observeTransition(const Track* prev, const Track* next, bool skipped);
    std::vector<RecommendResult> recommend(Library& lib, const Track& currentTrack, std::size_t limit = 10) const;
    
    const GenreGraphLearner& learner() const { return learner_; }
    GenreGraphLearner& learner() { return learner_; }
    void setLearner(const GenreGraphLearner& l);
};
