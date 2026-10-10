#pragma once

#include <string>
#include <vector>

#include "habit.h"

bool parseNumber(const std::string& text, int& value);
bool parseDay(const std::string& text, int today, int& day);
std::string padRight(const std::string& text, size_t width);

bool addHabit(std::vector<Habit>& habits, const std::string& name, int today);
bool removeHabit(std::vector<Habit>& habits, int id);
bool renameHabit(std::vector<Habit>& habits, int id, const std::string& name);
bool markDone(std::vector<Habit>& habits, int id, int day, int today);
bool unmarkDone(std::vector<Habit>& habits, int id, int day);
bool freezeDay(std::vector<Habit>& habits, int id, int day, int today);

void printList(const std::vector<Habit>& habits, int today);
void printLog(const std::vector<Habit>& habits, int today, int days);
void printStats(const std::vector<Habit>& habits, int today);

void runMenu(std::vector<Habit>& habits, const std::string& dataPath);
