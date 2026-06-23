#include "model/play_history.hpp"
#include "config.hpp"

void PlayHistory::push(const Track& t) {
    while (tail_ && tail_ != cursor_) {
        HistoryNode* temp = tail_;
        tail_ = tail_->prev;
        if (tail_) tail_->next = nullptr;
        delete temp;
        size_--;
    }

    HistoryNode* newNode = new HistoryNode{t.getMusicPath(), cursor_, nullptr};
    if (cursor_) {
        cursor_->next = newNode;
    } else {
        head_ = newNode;
    }
    tail_ = newNode;
    cursor_ = newNode;
    size_++;

    if (size_ > config::MAX_HISTORY) {
        HistoryNode* temp = head_;
        head_ = head_->next;
        if (head_) head_->prev = nullptr;
        delete temp;
        size_--;
    }
}

std::optional<fs::path> PlayHistory::back() {
    if (!canGoBack()) return std::nullopt;
    cursor_ = cursor_->prev;
    return cursor_->path;
}

std::optional<fs::path> PlayHistory::forward() {
    if (!canGoForward()) return std::nullopt;
    cursor_ = cursor_->next;
    return cursor_->path;
}

std::optional<fs::path> PlayHistory::current() const {
    if (!cursor_) return std::nullopt;
    return cursor_->path;
}

bool PlayHistory::canGoBack() const {
    return cursor_ && cursor_->prev;
}

bool PlayHistory::canGoForward() const {
    return cursor_ && cursor_->next;
}

std::vector<fs::path> PlayHistory::getHistory() const {
    std::vector<fs::path> vec;
    vec.reserve(size_);
    HistoryNode* curr = head_;
    while (curr) {
        vec.push_back(curr->path);
        curr = curr->next;
    }
    return vec;
}

void PlayHistory::clear() {
    HistoryNode* curr = head_;
    while (curr) {
        HistoryNode* temp = curr;
        curr = curr->next;
        delete temp;
    }
    head_ = tail_ = cursor_ = nullptr;
    size_ = 0;
}

PlayHistory::~PlayHistory() {
    clear();
}