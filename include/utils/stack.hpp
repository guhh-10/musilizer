#pragma once
#include <stdexcept>

template <typename T>
class Stack {
    struct Node { T data; Node* next = nullptr; };
    Node* top_ = nullptr;
    int   size_ = 0;
public:
    void push(const T& val) {
        Node* n = new Node{val, top_};
        top_ = n;
        size_++;
    }
    T pop() {
        if (!top_) throw std::underflow_error("Stack is empty");
        Node* n = top_;
        T val = n->data;
        top_ = top_->next;
        delete n;
        size_--;
        return val;
    }
    const T& peek() const {
        if (!top_) throw std::underflow_error("Stack is empty");
        return top_->data;
    }
    bool empty() const {
        return size_ == 0;
    }
    int size() const {
        return size_;
    }
    void clear() {
        while (top_) {
            Node* n = top_;
            top_ = top_->next;
            delete n;
        }
        size_ = 0;
    }
    ~Stack() {
        clear();
    }
};
