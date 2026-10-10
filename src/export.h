#pragma once

#include <string>
#include <vector>

#include "habit.h"

std::string csvField(const std::string& text);
std::string makeCsv(const std::vector<Habit>& habits, int today);
bool exportCsv(const std::string& path, const std::vector<Habit>& habits, int today);
