
#include "multirange.hpp"

void MultiRange::add_interval(int start, int end) {
    if (start >= end) {
        return;
    }

    auto iter = intervals.lower_bound(start);
    if (iter != intervals.begin()) {
        --iter;
        if (iter->second < start) {
            ++iter;
        } else {
            start = iter->first;
            end = std::max(end, iter->second);
            iter = intervals.erase(iter);
        }
    }

    while (iter != intervals.end() && iter->first <= end) {
        end = std::max(end, iter->second);
        iter = intervals.erase(iter);
    }

    intervals[start] = end;
}
void MultiRange::remove_interval(int start, int end) {
    if (start >= end) {
        return;
    }

    auto iter = intervals.lower_bound(start);
    if (iter != intervals.begin()) {
        --iter;
        if (iter->second <= start) {
            ++iter;
        } else {
            int old_start = iter->first;
            int old_end = iter->second;
            iter = intervals.erase(iter);
            if (old_start < start) {
                intervals[old_start] = start;
            }
            if (old_end > end) {
                intervals[end] = old_end;
                return;
            }
        }
    }

    while (iter != intervals.end() && iter->first < end) {
        int old_end = iter->second;
        iter = intervals.erase(iter);
        if (old_end > end) {
            intervals[end] = old_end;
            break;
        }
    }
}