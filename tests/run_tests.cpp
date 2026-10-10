#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "commands.h"
#include "date.h"
#include "export.h"
#include "habit.h"
#include "stats.h"
#include "storage.h"

int passed = 0;
std::vector<std::string> failures;

void check(bool condition, const std::string& name) {
    if (condition) {
        passed++;
    } else {
        failures.push_back(name);
    }
}

int day(const std::string& text) {
    Date date;
    parseDate(text, date);
    return dateToDays(date);
}

void testDates() {
    check(isLeapYear(2024), "2024 is leap");
    check(!isLeapYear(2026), "2026 is not leap");
    check(!isLeapYear(1900), "1900 is not leap");
    check(isLeapYear(2000), "2000 is leap");

    check(daysInMonth(2024, 2) == 29, "february 2024");
    check(daysInMonth(2026, 2) == 28, "february 2026");
    check(daysInMonth(2026, 4) == 30, "april");
    check(daysInMonth(2026, 12) == 31, "december");

    check(day("1970-01-01") == 0, "first day is 0");
    check(day("2000-03-01") == 11017, "2000-03-01");
    check(day("2024-02-29") == 19782, "2024-02-29");
    check(day("2026-10-09") == 20735, "2026-10-09");
    check(day("2027-01-01") - day("2026-12-31") == 1, "new year");
    check(day("2024-03-01") - day("2024-02-28") == 2, "leap february");

    bool roundTrip = true;
    for (int days = 0; days < 30000; days++) {
        if (dateToDays(daysToDate(days)) != days) {
            roundTrip = false;
        }
    }
    check(roundTrip, "days -> date -> days");

    check(formatDays(20735) == "2026-10-09", "format date");
    check(formatDays(0) == "1970-01-01", "format first day");

    check(weekdayName(day("1970-01-01")) == "Th", "1970-01-01 is thursday");
    check(weekdayName(day("2026-10-09")) == "Fr", "2026-10-09 is friday");
    check(weekdayName(day("2026-10-12")) == "Mo", "2026-10-12 is monday");

    Date date;
    check(parseDate("2026-10-09", date) && date.year == 2026 && date.month == 10 && date.day == 9, "parse date");
    check(!parseDate("2026-13-01", date), "month 13");
    check(!parseDate("2026-02-29", date), "29 february in a usual year");
    check(parseDate("2024-02-29", date), "29 february in a leap year");
    check(!parseDate("2026-04-31", date), "31 april");
    check(!parseDate("2026-1-5", date), "short date");
    check(!parseDate("09.10.2026", date), "wrong format");
    check(!parseDate("2026-10-0a", date), "letters");
    check(!parseDate("", date), "empty date");

    int result = 0;
    check(parseDay("today", 100, result) && result == 100, "today");
    check(parseDay("yesterday", 100, result) && result == 99, "yesterday");
    check(parseDay("2026-10-09", 100, result) && result == 20735, "date as day");
    check(!parseDay("tomorrow", 100, result), "unknown word");
}

void testStats() {
    int today = day("2026-10-09");

    Habit habit;
    habit.id = 1;
    habit.name = "Read";
    habit.created = day("2026-10-01");

    check(currentStreak(habit, today) == 0, "empty streak");
    check(bestStreak(habit) == 0, "empty best streak");
    check(percentDone(habit, today) == 0, "empty percent");
    check(totalDays(habit, today) == 9, "total days");

    habit.done.insert(day("2026-10-07"));
    habit.done.insert(day("2026-10-08"));
    habit.done.insert(day("2026-10-09"));
    check(currentStreak(habit, today) == 3, "streak with today");
    check(!streakInDanger(habit, today), "no danger when done today");

    habit.done.erase(day("2026-10-09"));
    check(currentStreak(habit, today) == 2, "streak without today");
    check(streakInDanger(habit, today), "danger when today is not done");

    habit.done.erase(day("2026-10-08"));
    check(currentStreak(habit, today) == 0, "streak is lost after a missed day");
    check(!streakInDanger(habit, today), "no danger without streak");

    habit.done.clear();
    habit.done.insert(day("2026-10-01"));
    habit.done.insert(day("2026-10-02"));
    habit.done.insert(day("2026-10-03"));
    habit.done.insert(day("2026-10-04"));
    habit.done.insert(day("2026-10-06"));
    habit.done.insert(day("2026-10-09"));
    check(bestStreak(habit) == 4, "best streak");
    check(currentStreak(habit, today) == 1, "current streak after a break");
    check(doneCount(habit, today) == 6, "done count");
    check(percentDone(habit, today) == 67, "percent 6 of 9");

    habit.done.clear();
    habit.done.insert(day("2026-09-30"));
    habit.done.insert(day("2026-10-01"));
    check(bestStreak(habit) == 2, "streak across months");
    check(firstDay(habit) == day("2026-09-30"), "mark before creation moves first day");
    check(totalDays(habit, today) == 10, "total days with early mark");

    habit.done.clear();
    habit.done.insert(day("2024-02-28"));
    habit.done.insert(day("2024-02-29"));
    habit.done.insert(day("2024-03-01"));
    check(bestStreak(habit) == 3, "streak across 29 february");

    habit.done.clear();
    habit.done.insert(day("2026-12-31"));
    habit.done.insert(day("2027-01-01"));
    check(currentStreak(habit, day("2027-01-01")) == 2, "streak across new year");
}

void testFreeze() {
    int today = day("2026-10-20");
    std::vector<Habit> habits;

    Habit habit;
    habit.id = 1;
    habit.name = "Read";
    habit.created = day("2026-10-01");
    for (int d = day("2026-10-01"); d <= day("2026-10-08"); d++) {
        habit.done.insert(d);
    }
    habit.done.insert(day("2026-10-10"));
    habit.done.insert(day("2026-10-11"));
    habits.push_back(habit);

    check(freezesEarned(habits[0], today) == 1, "one freeze for 10 done days");
    check(freezesLeft(habits[0], today) == 1, "freeze is not used yet");
    check(bestStreak(habits[0]) == 8, "best streak before freeze");

    check(!freezeDay(habits, 1, day("2026-10-08"), today), "cannot freeze a done day");
    check(!freezeDay(habits, 1, today + 1, today), "cannot freeze the future");
    check(!freezeDay(habits, 1, day("2026-09-30"), today), "cannot freeze before start");
    check(!freezeDay(habits, 5, day("2026-10-09"), today), "freeze unknown habit");

    check(freezeDay(habits, 1, day("2026-10-09"), today), "freeze a missed day");
    check(habits[0].frozen.count(day("2026-10-09")) == 1, "freeze is saved");
    check(bestStreak(habits[0]) == 10, "freeze joins two streaks");
    check(currentStreak(habits[0], day("2026-10-11")) == 10, "frozen day is not counted in streak");
    check(freezesLeft(habits[0], today) == 0, "freeze is used");
    check(freezeDay(habits, 1, day("2026-10-09"), today), "freeze twice is fine");
    check(habits[0].frozen.size() == 1, "no double freeze");
    check(!freezeDay(habits, 1, day("2026-10-12"), today), "no freezes left");

    check(currentStreak(habits[0], day("2026-10-12")) == 10, "streak waits for today");
    check(streakInDanger(habits[0], day("2026-10-12")), "danger before freeze");
    check(currentStreak(habits[0], day("2026-10-13")) == 0, "streak is lost without freeze");

    habits[0].frozen.insert(day("2026-10-12"));
    check(currentStreak(habits[0], day("2026-10-12")) == 10, "streak on a frozen today");
    check(!streakInDanger(habits[0], day("2026-10-12")), "no danger on a frozen today");
    check(currentStreak(habits[0], day("2026-10-13")) == 10, "streak after a frozen yesterday");
    check(streakInDanger(habits[0], day("2026-10-13")), "danger after a frozen yesterday");
    habits[0].frozen.erase(day("2026-10-12"));

    check(markDone(habits, 1, day("2026-10-09"), today), "done on a frozen day");
    check(habits[0].frozen.empty() && habits[0].done.count(day("2026-10-09")) == 1, "done replaces freeze");
    check(freezesLeft(habits[0], today) == 1, "freeze is returned");

    check(freezeDay(habits, 1, day("2026-10-12"), today), "freeze again");
    check(unmarkDone(habits, 1, day("2026-10-12")), "undo freeze");
    check(habits[0].frozen.empty(), "freeze is removed");

    Habit empty;
    empty.id = 2;
    empty.name = "Run";
    empty.created = day("2026-10-01");
    empty.frozen.insert(day("2026-10-05"));
    check(currentStreak(empty, day("2026-10-05")) == 0, "freeze alone gives no streak");
    check(bestStreak(empty) == 0, "freeze alone gives no best streak");
    check(freezesLeft(empty, today) == 0, "freezes left is never negative");
}

void testCommands() {
    int today = day("2026-10-09");
    std::vector<Habit> habits;

    check(addHabit(habits, "  Read book  ", today), "add habit");
    check(habits.size() == 1 && habits[0].name == "Read book" && habits[0].id == 1, "name is trimmed");
    check(!addHabit(habits, "Read book", today), "same name twice");
    check(!addHabit(habits, "   ", today), "empty name");
    check(addHabit(habits, "Run", today), "second habit");
    check(habits[1].id == 2, "second id");

    check(markDone(habits, 1, today, today), "mark done");
    check(habits[0].done.count(today) == 1, "mark is saved");
    check(markDone(habits, 1, today, today), "mark twice is fine");
    check(habits[0].done.size() == 1, "no double mark");
    check(!markDone(habits, 1, today + 1, today), "future day");
    check(!markDone(habits, 7, today, today), "unknown id");

    check(unmarkDone(habits, 1, today), "undo");
    check(habits[0].done.empty(), "mark is removed");

    check(renameHabit(habits, 2, "Morning run"), "rename");
    check(habits[1].name == "Morning run", "new name");

    check(removeHabit(habits, 1), "remove");
    check(habits.size() == 1 && habits[0].id == 2, "other habit stays");
    check(!removeHabit(habits, 1), "remove twice");
    check(addHabit(habits, "Sleep", today) && habits[1].id == 3, "new id after remove");

    check(padRight("abc", 5) == "abc  ", "pad latin");
    check(padRight("\xD0\xB0\xD0\xB1", 4) == "\xD0\xB0\xD0\xB1  ", "pad cyrillic");
}

void testStorage() {
    std::string path = (std::filesystem::temp_directory_path() / "habits_test_file.txt").string();
    std::filesystem::remove(path);

    std::vector<Habit> habits;
    std::string error;
    check(loadHabits(path, habits, error) && habits.empty(), "missing file is an empty list");

    Habit first;
    first.id = 1;
    first.name = "Read | 20 pages, \"daily\"";
    first.created = day("2026-10-01");
    first.done.insert(day("2026-10-01"));
    first.done.insert(day("2026-10-03"));
    first.frozen.insert(day("2026-10-02"));

    Habit second;
    second.id = 4;
    second.name = "\xD0\x91\xD0\xB5\xD0\xB3";
    second.created = day("2026-10-05");

    habits.push_back(first);
    habits.push_back(second);
    check(saveHabits(path, habits), "save");

    std::vector<Habit> loaded;
    check(loadHabits(path, loaded, error), "load");
    check(loaded.size() == 2, "two habits loaded");
    if (loaded.size() == 2) {
        check(loaded[0].id == 1 && loaded[0].name == first.name, "first habit");
        check(loaded[0].created == first.created && loaded[0].done == first.done, "first marks");
        check(loaded[0].frozen == first.frozen, "first freezes");
        check(loaded[1].id == 4 && loaded[1].name == second.name && loaded[1].done.empty(), "second habit");
    }
    check(nextHabitId(loaded) == 5, "next id");

    std::filesystem::remove(path);
}

void testCsv() {
    check(csvField("Read") == "Read", "simple field");
    check(csvField("Read, write") == "\"Read, write\"", "field with comma");
    check(csvField("Say \"hi\"") == "\"Say \"\"hi\"\"\"", "field with quotes");

    Habit habit;
    habit.id = 1;
    habit.name = "Read";
    habit.created = day("2026-10-07");
    habit.done.insert(day("2026-10-08"));
    habit.frozen.insert(day("2026-10-09"));

    std::vector<Habit> habits;
    habits.push_back(habit);

    std::string expected =
        "habit_id,habit,date,weekday,done,frozen\n"
        "1,Read,2026-10-07,We,0,0\n"
        "1,Read,2026-10-08,Th,1,0\n"
        "1,Read,2026-10-09,Fr,0,1\n";
    check(makeCsv(habits, day("2026-10-09")) == expected, "csv text");
}

int main() {
    std::streambuf* out = std::cout.rdbuf(nullptr);
    std::streambuf* err = std::cerr.rdbuf(nullptr);

    testDates();
    testStats();
    testFreeze();
    testCommands();
    testStorage();
    testCsv();

    std::cout.rdbuf(out);
    std::cerr.rdbuf(err);

    for (const std::string& name : failures) {
        std::cout << "[FAIL] " << name << "\n";
    }
    std::cout << "Passed: " << passed << ", failed: " << failures.size() << "\n";

    if (!failures.empty()) {
        return 1;
    }
    return 0;
}
