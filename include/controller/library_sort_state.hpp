#pragma once

#include "service/search.hpp"

enum class SortCycleState {
    NEUTRAL,
    ASC,
    DESC
};

class LibrarySortState {
public:
    void cycleColumn(int col);
    void applyTo(SearchQuery& q) const;

    int sortColumn() const { return sortCol_; }
    SortCycleState sortState() const { return sortState_; }

private:
    int sortCol_ = -1;
    SortCycleState sortState_ = SortCycleState::NEUTRAL;
};
