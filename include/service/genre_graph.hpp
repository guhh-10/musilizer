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

        // BFS: genres reachable from `start`, up to `maxDepth` hops, ordered by discovery
        std::vector<std::string> bfs(const std::string& start, int maxDepth = 2) const;

        // DFS: all genres reachable from `start` (no depth limit)
        std::vector<std::string> dfs(const std::string& start) const;

        void clear();
};