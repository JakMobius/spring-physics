#pragma once

#include <iostream>
#include <map>

class MultiRange {
  public:

    void add_interval(int start, int end);
    void remove_interval(int start, int end);

    const std::map<int, int>& get_intervals() const {
        return intervals;
    }

    int size() {
        int size = 0;
        for (auto& interval : intervals) {
            size += interval.second - interval.first;
        }
        return size;
    }

  private:
    std::map<int, int> intervals;
};