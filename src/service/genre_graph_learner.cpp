#include <algorithm>
#include <cctype>
#include <unordered_set>

#include "service/genre_graph_learner.hpp"
#include "utils/string_utils.hpp"

GenreGraphLearner::GenreGraphLearner(const GenreGraph& prior) {
    // Convert every edge in the prior graph to pseudo-counts.
    // prior_(A→B) = prior.weight(A→B) * PRIOR_STRENGTH
    for (const std::string& from : prior.nodes()) {
        for (const auto& [to, w] : prior.neighbors(from)) {
            if (w > 0.0f)
                prior_[utils::toLower(from)][utils::toLower(to)] = w * PRIOR_STRENGTH;
        }
    }
}

float GenreGraphLearner::rowSum(const CountTable& table, const std::string& from) {
    auto outer = table.find(from);
    if (outer == table.end()) return 0.0f;
    float sum = 0.0f;
    for (const auto& [_, v] : outer->second) sum += v;
    return sum;
}

void GenreGraphLearner::addObservation(
    const std::vector<std::string>& prevGenres,
    const std::vector<std::string>& nextGenres,
    float delta)
{
    for (const std::string& pg : prevGenres) {
        const std::string from = utils::toLower(pg);
        for (const std::string& ng : nextGenres) {
            const std::string to = utils::toLower(ng);
            float& cell = observed_[from][to];
            cell = std::max(0.0f, cell + delta);
        }
    }
    totalObs_++;
}

void GenreGraphLearner::observeCompletion(
    const std::vector<std::string>& prevGenres,
    const std::vector<std::string>& nextGenres)
{
    addObservation(prevGenres, nextGenres, +1.0f);
}

void GenreGraphLearner::observeSkip(
    const std::vector<std::string>& prevGenres,
    const std::vector<std::string>& nextGenres)
{
    addObservation(prevGenres, nextGenres, -SKIP_PENALTY);
}

GenreGraph GenreGraphLearner::toGraph(float minWeight) const {
    // Collect all (from, to) pairs across both tables.
    std::unordered_map<std::string, std::unordered_set<std::string>> allPairs;

    auto collectPairs = [&](const CountTable& t) {
        for (const auto& [from, tos] : t)
            for (const auto& [to, _] : tos)
                allPairs[from].insert(to);
    };
    collectPairs(prior_);
    collectPairs(observed_);

    GenreGraph g;

    for (const auto& [from, tos] : allPairs) {
        const float priorTotal    = rowSum(prior_,    from);
        const float observedTotal = rowSum(observed_, from);
        const float denom         = priorTotal + observedTotal;

        if (denom == 0.0f) continue;

        for (const std::string& to : tos) {
            float pCount = 0.0f;
            if (auto outer = prior_.find(from); outer != prior_.end())
                if (auto inner = outer->second.find(to); inner != outer->second.end())
                    pCount = inner->second;

            float oCount = 0.0f;
            if (auto outer = observed_.find(from); outer != observed_.end())
                if (auto inner = outer->second.find(to); inner != outer->second.end())
                    oCount = inner->second;

            const float weight = (pCount + oCount) / denom;
            if (weight >= minWeight)
                g.addEdge(from, to, weight);
        }
    }

    return g;
}

const GenreGraphLearner::CountTable& GenreGraphLearner::observedCounts() const {
    return observed_;
}

const GenreGraphLearner::CountTable& GenreGraphLearner::priorCounts() const {
    return prior_;
}

void GenreGraphLearner::setObservedCounts(CountTable counts) {
    observed_ = std::move(counts);
    // Recount totalObs_ from the loaded data.
    totalObs_ = 0;
    for (const auto& [_, tos] : observed_)
        for (const auto& [__, v] : tos)
            if (v > 0.0f) totalObs_++;
}

int GenreGraphLearner::totalObservations() const {
    return totalObs_;
}