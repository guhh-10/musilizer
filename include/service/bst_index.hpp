#pragma once
#include <string>
#include <vector>
#include <set>
#include <filesystem>

namespace fs = std::filesystem;

struct BSTNode {
    std::string           key;
    std::vector<fs::path> paths;
    BSTNode*              left  = nullptr;
    BSTNode*              right = nullptr;
};

class BSTIndex {
    BSTNode* root_ = nullptr;

    BSTNode* insert(BSTNode* node, const std::string& key, const fs::path& path);
    void     collect(BSTNode* node, const std::string& prefix, std::set<fs::path>& out) const;
    void     clear(BSTNode* node);

public:
    void insert(const std::string& key, const fs::path& path);
    std::set<fs::path> prefixSearch(const std::string& prefix) const;
    void clear();
    ~BSTIndex() { clear(); }
};
