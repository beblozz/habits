#pragma once

#include <string>
#include <vector>

#include "habit.h"

bool loadHabits(const std::string& path, std::vector<Habit>& habits, std::string& error);
bool saveHabits(const std::string& path, const std::vector<Habit>& habits);
int findHabit(const std::vector<Habit>& habits, int id);
int nextHabitId(const std::vector<Habit>& habits);
