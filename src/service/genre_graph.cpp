#include <set>
#include <queue>
#include <stack>

#include "service/genre_graph.hpp"

void GenreGraph::addEdge(const std::string& from, const std::string& to, float weight) {
    adj_[from][to] = weight;
}

float GenreGraph::weight(const std::string& from, const std::string& to) const {
    auto outer = adj_.find(from);
    if (outer == adj_.end()) return 0.0f;
    auto inner = outer->second.find(to);
    if (inner == outer->second.end()) return 0.0f;
    return inner->second;
}

std::vector<std::pair<std::string, float>>
GenreGraph::neighbors(const std::string& from) const {
    auto outer = adj_.find(from);
    if (outer == adj_.end()) return {};
    std::vector<std::pair<std::string, float>> result;
    result.reserve(outer->second.size());
    for (const auto& [to, w] : outer->second)
        result.emplace_back(to, w);
    return result;
}

std::vector<std::string> GenreGraph::nodes() const {
    std::set<std::string> seen;
    for (const auto& [from, tos] : adj_) {
        seen.insert(from);
        for (const auto& [to, _] : tos)
            seen.insert(to);
    }
    return {seen.begin(), seen.end()};
}

void GenreGraph::clear() {
    adj_.clear();
}

std::vector<std::string> GenreGraph::bfs(const std::string& start, int maxDepth) const {
    std::vector<std::string> result;
    std::set<std::string> visited;
    std::queue<std::pair<std::string, int>> q;

    q.push({start, 0});
    visited.insert(start);

    while (!q.empty()) {
        auto [curr, depth] = q.front();
        q.pop();

        if (curr != start) {
            result.push_back(curr);
        }

        if (depth < maxDepth) {
            for (const auto& [next, weight] : neighbors(curr)) {
                if (visited.find(next) == visited.end()) {
                    visited.insert(next);
                    q.push({next, depth + 1});
                }
            }
        }
    }
    return result;
}

std::vector<std::string> GenreGraph::dfs(const std::string& start) const {
    std::vector<std::string> result;
    std::set<std::string> visited;
    std::stack<std::string> s;

    s.push(start);

    while (!s.empty()) {
        std::string curr = s.top();
        s.pop();

        if (visited.find(curr) == visited.end()) {
            visited.insert(curr);
            if (curr != start) {
                result.push_back(curr);
            }

            for (const auto& [next, weight] : neighbors(curr)) {
                if (visited.find(next) == visited.end()) {
                    s.push(next);
                }
            }
        }
    }
    return result;
}