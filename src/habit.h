#pragma once

#include <set>
#include <string>

struct Habit {
    int id = 0;
    std::string name;
    int created = 0;
    std::set<int> done;
    std::set<int> frozen;
};
