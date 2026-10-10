#pragma once

#include "habit.h"

const int DAYS_FOR_FREEZE = 7;

int firstDay(const Habit& habit);
int totalDays(const Habit& habit, int today);
int doneCount(const Habit& habit, int today);
int percentDone(const Habit& habit, int today);
int currentStreak(const Habit& habit, int today);
int bestStreak(const Habit& habit);
bool streakInDanger(const Habit& habit, int today);
int freezesEarned(const Habit& habit, int today);
int freezesLeft(const Habit& habit, int today);
