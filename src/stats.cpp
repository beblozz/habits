#include "stats.h"

int firstDay(const Habit& habit) {
    if (!habit.done.empty() && *habit.done.begin() < habit.created) {
        return *habit.done.begin();
    }
    return habit.created;
}

int totalDays(const Habit& habit, int today) {
    int days = today - firstDay(habit) + 1;
    if (days < 1) {
        return 1;
    }
    return days;
}

int doneCount(const Habit& habit, int today) {
    int count = 0;
    for (int day : habit.done) {
        if (day <= today) {
            count++;
        }
    }
    return count;
}

int percentDone(const Habit& habit, int today) {
    int total = totalDays(habit, today);
    int done = doneCount(habit, today);
    return (done * 100 + total / 2) / total;
}

static bool isKept(const Habit& habit, int day) {
    return habit.done.count(day) > 0 || habit.frozen.count(day) > 0;
}

int currentStreak(const Habit& habit, int today) {
    int day = today;
    if (!isKept(habit, day)) {
        day--;
    }

    int streak = 0;
    while (isKept(habit, day)) {
        if (habit.done.count(day) > 0) {
            streak++;
        }
        day--;
    }
    return streak;
}

int bestStreak(const Habit& habit) {
    if (habit.done.empty()) {
        return 0;
    }

    int first = *habit.done.begin();
    int last = *habit.done.rbegin();
    int best = 0;
    int current = 0;

    for (int day = first; day <= last; day++) {
        if (habit.done.count(day) > 0) {
            current++;
            if (current > best) {
                best = current;
            }
        } else if (habit.frozen.count(day) == 0) {
            current = 0;
        }
    }
    return best;
}

bool streakInDanger(const Habit& habit, int today) {
    return !isKept(habit, today) && currentStreak(habit, today) > 0;
}

int freezesEarned(const Habit& habit, int today) {
    return doneCount(habit, today) / DAYS_FOR_FREEZE;
}

int freezesLeft(const Habit& habit, int today) {
    int left = freezesEarned(habit, today) - static_cast<int>(habit.frozen.size());
    if (left < 0) {
        return 0;
    }
    return left;
}
