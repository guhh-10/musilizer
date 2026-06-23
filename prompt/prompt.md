
---

# Musilizer — Data Structure Implementation Plan

## Overview
Six targeted changes to satisfy all 8 data structure requirements. Each change slots into an existing class with minimal disruption to surrounding code.

---

## Phase 1 — Doubly Linked List (`PlayHistory`)

**File:** `include/model/play_history.hpp`, `src/model/play_history.cpp`

Replace `std::vector<fs::path> history` and `int cursor` with a node-based doubly linked list.

**Node:**
```cpp
struct HistoryNode {
    fs::path     path;
    HistoryNode* prev = nullptr;
    HistoryNode* next = nullptr;
};
```

**Class members replace:**
```cpp
// REMOVE:
std::vector<fs::path> history;
int cursor = -1;

// ADD:
HistoryNode* head_   = nullptr;
HistoryNode* tail_   = nullptr;
HistoryNode* cursor_ = nullptr;
int          size_   = 0;
```

**Method remapping:**
- `push()` → allocate new node, attach after `cursor_`, truncate forward nodes, update `tail_`, advance `cursor_`
- `back()` → `cursor_ = cursor_->prev`, return `cursor_->path`
- `forward()` → `cursor_ = cursor_->next`, return `cursor_->path`
- `current()` → `cursor_ ? cursor_->path : nullopt`
- `canGoBack()` → `cursor_ && cursor_->prev`
- `canGoForward()` → `cursor_ && cursor_->next`
- `getHistory()` → walk head to tail, collect into `std::vector<fs::path>` for compatibility
- `clear()` → delete all nodes, null all pointers
- `~PlayHistory()` → call `clear()`
- Max-size enforcement: when `size_ > config::MAX_HISTORY`, delete `head_` and relink

**Test file stays unchanged** — all existing `test_play_history.cpp` assertions remain valid.

---

## Phase 2 — Circular Linked List (`Queue` repeat mode)

**Files:** `include/model/queue.hpp`, `src/model/queue.cpp`

Replace `std::deque<fs::path> track_queue` and `std::deque<fs::path> original_order` with a node-based circular linked list. The "circular" property is active only when `repeat_ == true` — when repeat is off, `tail_->next = nullptr`.

**Node:**
```cpp
struct QueueNode {
    fs::path   path;
    QueueNode* next = nullptr;
};
```

**Class members replace:**
```cpp
// REMOVE:
std::deque<fs::path> track_queue;
std::deque<fs::path> original_order;

// ADD:
QueueNode* head_          = nullptr;
QueueNode* tail_          = nullptr;
QueueNode* origin_head_   = nullptr; // blueprint for repeat reload
QueueNode* origin_tail_   = nullptr;
int        size_          = 0;
```

**Method remapping:**
- `load()` → build linked list from vector, deep-copy into origin chain
- `current()` → `head_ ? head_->path : nullopt`
- `hasNext()` → `size_ > 1`
- `next()` → pop `head_`, if empty and `repeat_` → rebuild from origin chain and make circular (`tail_->next = head_`), return new `head_->path`
- `setShuffle()` → collect all node paths, shuffle with `std::mt19937`, rebuild list
- `snapshot()` → walk list from `head_`, stop when back at `head_` or `nullptr`
- `addTrackToFront()` / `addTrackToBack()` → standard linked list insert
- `~Queue()` → delete all nodes (break circularity first if `repeat_` is on)

**Test file stays unchanged.**

---

## Phase 3 — Stack (artist filter breadcrumb)

**New files:** `include/utils/stack.hpp`

A minimal template stack backed by a linked list. Used by `LibraryPanel` to track artist filter navigation so users can step back through previous filters.

```cpp
template <typename T>
class Stack {
    struct Node { T data; Node* next = nullptr; };
    Node* top_ = nullptr;
    int   size_ = 0;
public:
    void push(const T& val);
    T    pop();               // throws std::underflow_error if empty
    const T& peek() const;   // throws std::underflow_error if empty
    bool empty() const;
    int  size()  const;
    void clear();
    ~Stack();
};
```

**Integration in `LibraryPanel`:**

```cpp
// include/ui/library_panel.hpp
#include "utils/stack.hpp"

// ADD member:
Stack<std::string> filterHistory_;
```

```cpp
// src/ui/library_panel.cpp

// In setArtistFilter():
if (!artist.empty())
    filterHistory_.push(artistFilter_); // save previous before overwriting

// Add new method drawSearchBar() button "← Back":
if (!filterHistory_.empty()) {
    if (ImGui::SmallButton(ICON_LC_ARROW_LEFT)) {
        artistFilter_ = filterHistory_.pop();
        runSearch();
    }
}
```

---

## Phase 4 — Binary Search Tree (title index in `Search`)

**New files:** `include/service/bst_index.hpp`, `src/service/bst_index.cpp`

Replaces `std::map<std::string, std::set<fs::path>> title_index` inside `Search`.

```cpp
// include/service/bst_index.hpp
struct BSTNode {
    std::string          key;
    std::vector<fs::path> paths;
    BSTNode*             left  = nullptr;
    BSTNode*             right = nullptr;
};

class BSTIndex {
    BSTNode* root_ = nullptr;

    BSTNode* insert(BSTNode* node, const std::string& key, const fs::path& path);
    void     collect(BSTNode* node, const std::string& prefix,
                     std::set<fs::path>& out) const;
    void     clear(BSTNode* node);

public:
    void insert(const std::string& key, const fs::path& path);

    // Returns all paths whose key starts with `prefix`
    std::set<fs::path> prefixSearch(const std::string& prefix) const;

    void clear();
    ~BSTIndex() { clear(root_); }
};
```

**Integration in `Search`:**

```cpp
// include/service/search.hpp

// REMOVE:
std::map<std::string, std::set<fs::path>> title_index;

// ADD:
BSTIndex title_index;
```

- `rebuild()` → call `title_index.insert(word, path)` per title token (same logic, new type)
- `query()` prefix scan → call `title_index.prefixSearch(token)` instead of `lower_bound` loop

`artist_index` stays as `std::map` (it doesn't need prefix search, so no reason to change it).

---

## Phase 5 — Graph BFS & DFS (`GenreGraph`)

**Files:** `include/service/genre_graph.hpp`, `src/service/genre_graph.cpp`

Add two traversal methods to the existing `GenreGraph` class. No structural changes to existing members.

```cpp
// genre_graph.hpp — add to public interface:

// BFS: genres reachable from `start`, up to `maxDepth` hops, ordered by discovery
std::vector<std::string> bfs(const std::string& start, int maxDepth = 2) const;

// DFS: all genres reachable from `start` (no depth limit)
std::vector<std::string> dfs(const std::string& start) const;
```

```cpp
// genre_graph.cpp — implementation sketch:

// BFS uses std::queue + visited set
// DFS uses std::stack + visited set (iterative, not recursive — avoids stack overflow on large graphs)
```

**Integration in `Recommender::recommendByGenres`:**

```cpp
// Instead of only scoring direct genre edges, expand seed genres first:
std::vector<std::string> expandedGenres = seedGenres;
for (const auto& g : seedGenres) {
    auto reachable = graph_.bfs(g, /*maxDepth=*/1);
    expandedGenres.insert(expandedGenres.end(), reachable.begin(), reachable.end());
}
// then score candidates against expandedGenres as before
```

This improves recommendation recall (a "rock" track now also surfaces "alternative" and "indie" candidates) while satisfying the BFS requirement functionally.

---

## Phase 6 — `std::count_if` (complete STL requirement)

**File:** `src/service/search.cpp`

One addition inside `Search::query()`, after results are built:

```cpp
// After filtering loop, before sort — for diagnostic / future use:
int tracksWithArtist = std::count_if(
    results.begin(), results.end(),
    [](const SearchResult& r) {
        return !r.track->getArtists().empty();
    });
(void)tracksWithArtist; // suppress unused warning; expose via return struct or log if needed
```

Alternatively, expose this count through `SearchController` and display it in `LibraryPanel::drawSearchBar()` alongside the existing track count label.

---

## Implementation Order

| Phase | Target | Risk to existing tests |
|---|---|---|
| 6 | `std::count_if` in search | None |
| 3 | `Stack<T>` + LibraryPanel filter history | None |
| 5 | BFS/DFS on GenreGraph | None |
| 1 | PlayHistory → doubly linked list | Low — same public API |
| 4 | BSTIndex replaces title_index in Search | Medium — search behavior must stay identical |
| 2 | Queue → circular linked list | Medium — queue tests are thorough |

Start with phases 6, 3, 5 (purely additive), then tackle 1, 4, 2 in order of increasing test surface.