#include "service/recommendation_coordinator.hpp"

RecommendationCoordinator::RecommendationCoordinator()
    : learner_(Recommender::buildDefaultGraph())
    , recommender_(learner_.toGraph())
{}

void RecommendationCoordinator::observeTransition(const Track* prev, const Track* next, bool skipped) {
    if (!prev || !next) return;
    if (prev->getGenres().empty() || next->getGenres().empty()) return;
    if (skipped)
        learner_.observeSkip(prev->getGenres(), next->getGenres());
    else
        learner_.observeCompletion(prev->getGenres(), next->getGenres());
    recommender_.setGraph(learner_.toGraph());
    
    if (onRecommendationsReady) {
        onRecommendationsReady();
    }
}

std::vector<RecommendResult> RecommendationCoordinator::recommend(Library& lib, const Track& currentTrack, std::size_t limit) const {
    return recommender_.recommend(lib, currentTrack, limit);
}

void RecommendationCoordinator::setLearner(const GenreGraphLearner& l) {
    learner_ = l;
    recommender_.setGraph(learner_.toGraph());
}
