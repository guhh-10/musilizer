#include "controller/library_sort_state.hpp"

void LibrarySortState::cycleColumn(int col)
{
    if (sortCol_ != col) {
        sortCol_ = col;
        sortState_ = SortCycleState::ASC;
        return;
    }

    switch (sortState_) {
        case SortCycleState::NEUTRAL:
            sortState_ = SortCycleState::ASC;
            break;
        case SortCycleState::ASC:
            sortState_ = SortCycleState::DESC;
            break;
        case SortCycleState::DESC:
            sortState_ = SortCycleState::NEUTRAL;
            sortCol_ = -1;
            break;
    }
}

void LibrarySortState::applyTo(SearchQuery& q) const
{
    if (sortCol_ == -1 || sortState_ == SortCycleState::NEUTRAL) {
        return;
    }

    q.sortOrder = (sortState_ == SortCycleState::ASC) ? SortOrder::ASC : SortOrder::DESC;

    switch (sortCol_) {
        case 1:
            q.sortBy = SortField::TITLE;
            break;
        case 2:
            q.sortBy = SortField::ARTIST;
            break;
        case 3:
            q.sortBy = SortField::DURATION;
            break;
        default:
            break;
    }
}
