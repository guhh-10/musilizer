#pragma once
#include <string>
#include <unordered_map>
#include <vector>

// Weights are in [0.0f, 1.0f].
class GenreGraph {
    private:
        // adj_[from][to] = weight
        std::unordered_map<std::string,
        std::unordered_map<std::string, float>> adj_;

    public:
        void addEdge(const std::string& from, const std::string& to, float weight);
        float weight(const std::string& from, const std::string& to) const;

        // All genres that have at least one outgoing edge from `from`.
        // Returns empty vector if `from` is not in the graph.
        std::vector<std::pair<std::string, float>> neighbors(const std::string& from) const;
        // All genre nodes
        std::vector<std::string> nodes() const;

        void clear();
};