#include <deque>
#include <algorithm>
#include <random>

#include "model/queue.hpp"

void Queue::clearQueue() {
    if (repeat_ && tail_) tail_->next = nullptr;
    while (head_) {
        QueueNode* t = head_;
        head_ = head_->next;
        delete t;
    }
    head_ = tail_ = nullptr;
    size_ = 0;
}

void Queue::clearOrigin() {
    while (origin_head_) {
        QueueNode* t = origin_head_;
        origin_head_ = origin_head_->next;
        delete t;
    }
    origin_head_ = origin_tail_ = nullptr;
}

void Queue::rebuildFromOrigin() {
    clearQueue();
    QueueNode* curr = origin_head_;
    while (curr) {
        QueueNode* n = new QueueNode{curr->path, nullptr};
        if (!head_) { head_ = tail_ = n; }
        else { tail_->next = n; tail_ = n; }
        size_++;
        curr = curr->next;
    }
}

void Queue::makeCircularIfRepeat() {
    if (repeat_ && tail_ && head_) {
        tail_->next = head_;
    } else if (!repeat_ && tail_) {
        tail_->next = nullptr;
    }
}

void Queue::load(const std::vector<const Track*>& tracks) {
    clearQueue();
    clearOrigin();
    for (const Track* t : tracks) {
        QueueNode* n1 = new QueueNode{t->getMusicPath(), nullptr};
        if (!head_) { head_ = tail_ = n1; }
        else { tail_->next = n1; tail_ = n1; }
        size_++;

        QueueNode* n2 = new QueueNode{t->getMusicPath(), nullptr};
        if (!origin_head_) { origin_head_ = origin_tail_ = n2; }
        else { origin_tail_->next = n2; origin_tail_ = n2; }
    }
    makeCircularIfRepeat();
}

std::optional<fs::path> Queue::current() const {
    if (!head_) return std::nullopt;
    return head_->path;
}

bool Queue::hasNext() const {
    return size_ > 1;
}

std::optional<fs::path> Queue::next() {
    if (!head_) return std::nullopt;

    QueueNode* old = head_;
    if (tail_ == head_) tail_ = nullptr;
    head_ = head_->next;
    size_--;

    if (repeat_ && tail_) {
        tail_->next = head_;
    } else if (tail_) {
        tail_->next = nullptr;
    }
    delete old;

    if (!head_) {
        if (!repeat_) return std::nullopt;
        rebuildFromOrigin();
        if (shuffle_ && size_ > 1) {
            std::vector<fs::path> paths;
            QueueNode* curr = head_;
            while (curr) { paths.push_back(curr->path); curr = curr->next; }
            std::shuffle(paths.begin(), paths.end(), std::mt19937{std::random_device{}()});
            clearQueue();
            for (const auto& p : paths) {
                QueueNode* n = new QueueNode{p, nullptr};
                if (!head_) { head_ = tail_ = n; }
                else { tail_->next = n; tail_ = n; }
                size_++;
            }
        }
        makeCircularIfRepeat();
    }

    if (!head_) return std::nullopt;
    return head_->path;
}

void Queue::setShuffle(bool enabled) {
    shuffle_ = enabled;
    if (!head_) return;

    if (shuffle_) {
        std::vector<fs::path> paths;
        fs::path cur = head_->path;
        QueueNode* curr = head_->next;
        if (repeat_ && tail_) tail_->next = nullptr;
        while (curr) { paths.push_back(curr->path); curr = curr->next; }
        
        if (paths.size() > 0) {
            std::shuffle(paths.begin(), paths.end(), std::mt19937{std::random_device{}()});
        }
        
        clearQueue();
        QueueNode* n = new QueueNode{cur, nullptr};
        head_ = tail_ = n;
        size_ = 1;
        for (const auto& p : paths) {
            QueueNode* nn = new QueueNode{p, nullptr};
            tail_->next = nn;
            tail_ = nn;
            size_++;
        }
        makeCircularIfRepeat();
    } else {
        fs::path cur = head_->path;
        rebuildFromOrigin();
        
        std::vector<fs::path> paths;
        QueueNode* curr = head_;
        while (curr) { paths.push_back(curr->path); curr = curr->next; }
        
        auto it = std::find(paths.begin(), paths.end(), cur);
        if (it != paths.end()) {
            std::rotate(paths.begin(), it, paths.end());
        }
        
        clearQueue();
        for (const auto& p : paths) {
            QueueNode* n = new QueueNode{p, nullptr};
            if (!head_) { head_ = tail_ = n; }
            else { tail_->next = n; tail_ = n; }
            size_++;
        }
        makeCircularIfRepeat();
    }
}

void Queue::setRepeat(bool enabled) {
    repeat_ = enabled;
    makeCircularIfRepeat();
}

void Queue::addTrackToFront(const Track& t) {
    fs::path path = t.getMusicPath();
    QueueNode* n = new QueueNode{path, nullptr};
    if (!head_) {
        head_ = tail_ = n;
    } else {
        n->next = head_->next;
        head_->next = n;
        if (tail_ == head_) tail_ = n;
    }
    size_++;
    makeCircularIfRepeat();

    QueueNode* on = new QueueNode{path, nullptr};
    if (!origin_head_) {
        origin_head_ = origin_tail_ = on;
    } else {
        on->next = origin_head_->next;
        origin_head_->next = on;
        if (origin_tail_ == origin_head_) origin_tail_ = on;
    }
}

void Queue::addTrackToBack(const Track& t) {
    fs::path path = t.getMusicPath();
    QueueNode* n = new QueueNode{path, nullptr};
    if (!head_) {
        head_ = tail_ = n;
    } else {
        tail_->next = n;
        tail_ = n;
    }
    size_++;
    makeCircularIfRepeat();

    QueueNode* on = new QueueNode{path, nullptr};
    if (!origin_head_) {
        origin_head_ = origin_tail_ = on;
    } else {
        origin_tail_->next = on;
        origin_tail_ = on;
    }
}

std::vector<fs::path> Queue::snapshot() const {
    std::vector<fs::path> res;
    QueueNode* curr = head_;
    while (curr) {
        res.push_back(curr->path);
        curr = curr->next;
        if (curr == head_) break;
    }
    return res;
}

bool Queue::isShuffle() const { return shuffle_; }
bool Queue::isRepeat() const { return repeat_; }

Queue::~Queue() {
    clearQueue();
    clearOrigin();
}