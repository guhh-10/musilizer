#include "service/bst_index.hpp"

BSTNode* BSTIndex::insert(BSTNode* node, const std::string& key, const fs::path& path) {
    if (!node) {
        BSTNode* n = new BSTNode;
        n->key = key;
        n->paths.push_back(path);
        return n;
    }
    if (key < node->key) {
        node->left = insert(node->left, key, path);
    } else if (key > node->key) {
        node->right = insert(node->right, key, path);
    } else {
        node->paths.push_back(path);
    }
    return node;
}

void BSTIndex::insert(const std::string& key, const fs::path& path) {
    root_ = insert(root_, key, path);
}

void BSTIndex::collect(BSTNode* node, const std::string& prefix, std::set<fs::path>& out) const {
    if (!node) return;

    bool isPrefix = (node->key.size() >= prefix.size() && node->key.compare(0, prefix.size(), prefix) == 0);

    if (isPrefix) {
        out.insert(node->paths.begin(), node->paths.end());
        collect(node->left, prefix, out);
        collect(node->right, prefix, out);
    } else if (node->key > prefix) {
        collect(node->left, prefix, out);
    } else {
        collect(node->right, prefix, out);
    }
}

std::set<fs::path> BSTIndex::prefixSearch(const std::string& prefix) const {
    std::set<fs::path> out;
    collect(root_, prefix, out);
    return out;
}

void BSTIndex::clear(BSTNode* node) {
    if (!node) return;
    clear(node->left);
    clear(node->right);
    delete node;
}

void BSTIndex::clear() {
    clear(root_);
    root_ = nullptr;
}
