#include "commands.h"

#include <iostream>

#include "date.h"
#include "export.h"
#include "stats.h"
#include "storage.h"

bool parseNumber(const std::string& text, int& value) {
    if (text.empty() || text.size() > 6) {
        return false;
    }
    for (char c : text) {
        if (c < '0' || c > '9') {
            return false;
        }
    }
    value = std::stoi(text);
    return true;
}

bool parseDay(const std::string& text, int today, int& day) {
    if (text == "today") {
        day = today;
        return true;
    }
    if (text == "yesterday") {
        day = today - 1;
        return true;
    }
    Date date;
    if (!parseDate(text, date)) {
        return false;
    }
    day = dateToDays(date);
    return true;
}

static size_t textWidth(const std::string& text) {
    size_t width = 0;
    for (unsigned char c : text) {
        if ((c & 0xC0) != 0x80) {
            width++;
        }
    }
    return width;
}

std::string padRight(const std::string& text, size_t width) {
    std::string result = text;
    size_t current = textWidth(text);
    while (current < width) {
        result += " ";
        current++;
    }
    return result;
}

static std::string padLeft(const std::string& text, size_t width) {
    std::string result = text;
    while (textWidth(result) < width) {
        result = " " + result;
    }
    return result;
}

static std::string cleanName(const std::string& name) {
    std::string result;
    for (char c : name) {
        if (c == '\n' || c == '\r' || c == '\t') {
            result += ' ';
        } else {
            result += c;
        }
    }
    size_t first = result.find_first_not_of(' ');
    if (first == std::string::npos) {
        return "";
    }
    size_t last = result.find_last_not_of(' ');
    return result.substr(first, last - first + 1);
}

static size_t nameColumnWidth(const std::vector<Habit>& habits) {
    size_t width = 5;
    for (const Habit& habit : habits) {
        if (textWidth(habit.name) > width) {
            width = textWidth(habit.name);
        }
    }
    return width;
}

static std::string daysWord(int count) {
    if (count == 1) {
        return "1 day";
    }
    return std::to_string(count) + " days";
}

bool addHabit(std::vector<Habit>& habits, const std::string& name, int today) {
    std::string clean = cleanName(name);
    if (clean.empty()) {
        std::cerr << "Habit name is empty\n";
        return false;
    }
    for (const Habit& habit : habits) {
        if (habit.name == clean) {
            std::cerr << "Habit \"" << clean << "\" already exists\n";
            return false;
        }
    }

    Habit habit;
    habit.id = nextHabitId(habits);
    habit.name = clean;
    habit.created = today;
    habits.push_back(habit);

    std::cout << "Added habit " << habit.id << ": " << habit.name << "\n";
    return true;
}

bool removeHabit(std::vector<Habit>& habits, int id) {
    int index = findHabit(habits, id);
    if (index == -1) {
        std::cerr << "No habit with id " << id << "\n";
        return false;
    }
    std::cout << "Removed habit " << id << ": " << habits[index].name << "\n";
    habits.erase(habits.begin() + index);
    return true;
}

bool renameHabit(std::vector<Habit>& habits, int id, const std::string& name) {
    int index = findHabit(habits, id);
    if (index == -1) {
        std::cerr << "No habit with id " << id << "\n";
        return false;
    }
    std::string clean = cleanName(name);
    if (clean.empty()) {
        std::cerr << "Habit name is empty\n";
        return false;
    }
    habits[index].name = clean;
    std::cout << "Renamed habit " << id << ": " << clean << "\n";
    return true;
}

bool markDone(std::vector<Habit>& habits, int id, int day, int today) {
    int index = findHabit(habits, id);
    if (index == -1) {
        std::cerr << "No habit with id " << id << "\n";
        return false;
    }
    if (day > today) {
        std::cerr << "Cannot mark a day in the future\n";
        return false;
    }

    Habit& habit = habits[index];
    if (habit.done.count(day) > 0) {
        std::cout << habit.name << ": " << formatDays(day) << " is already marked\n";
        return true;
    }

    habit.done.insert(day);
    habit.frozen.erase(day);
    std::cout << habit.name << ": done on " << formatDays(day)
              << ", streak " << daysWord(currentStreak(habit, today)) << "\n";
    return true;
}

bool unmarkDone(std::vector<Habit>& habits, int id, int day) {
    int index = findHabit(habits, id);
    if (index == -1) {
        std::cerr << "No habit with id " << id << "\n";
        return false;
    }

    Habit& habit = habits[index];
    if (habit.done.erase(day) > 0) {
        std::cout << habit.name << ": mark removed for " << formatDays(day) << "\n";
    } else if (habit.frozen.erase(day) > 0) {
        std::cout << habit.name << ": freeze removed for " << formatDays(day) << "\n";
    } else {
        std::cout << habit.name << ": " << formatDays(day) << " was not marked\n";
    }
    return true;
}

bool freezeDay(std::vector<Habit>& habits, int id, int day, int today) {
    int index = findHabit(habits, id);
    if (index == -1) {
        std::cerr << "No habit with id " << id << "\n";
        return false;
    }

    Habit& habit = habits[index];
    if (day > today) {
        std::cerr << "Cannot freeze a day in the future\n";
        return false;
    }
    if (day < firstDay(habit)) {
        std::cerr << "Cannot freeze a day before the habit was started\n";
        return false;
    }
    if (habit.done.count(day) > 0) {
        std::cerr << habit.name << ": " << formatDays(day) << " is done, no need to freeze it\n";
        return false;
    }
    if (habit.frozen.count(day) > 0) {
        std::cout << habit.name << ": " << formatDays(day) << " is already frozen\n";
        return true;
    }
    if (freezesLeft(habit, today) == 0) {
        std::cerr << habit.name << ": no freezes left, you get one for every "
                  << DAYS_FOR_FREEZE << " done days\n";
        return false;
    }

    habit.frozen.insert(day);
    std::cout << habit.name << ": " << formatDays(day) << " is frozen, streak "
              << daysWord(currentStreak(habit, today)) << ", freezes left: "
              << freezesLeft(habit, today) << "\n";
    return true;
}

void printList(const std::vector<Habit>& habits, int today) {
    if (habits.empty()) {
        std::cout << "No habits yet. Add one: habits add \"Read 20 pages\"\n";
        return;
    }

    size_t width = nameColumnWidth(habits);
    bool danger = false;
    int doneToday = 0;

    std::cout << "Today is " << formatDays(today) << " (" << weekdayName(today) << ")\n\n";
    std::cout << " ID  Today  " << padRight("Habit", width) << "  Freezes  Streak\n";

    for (const Habit& habit : habits) {
        bool done = habit.done.count(today) > 0;
        if (done) {
            doneToday++;
        }

        std::cout << padLeft(std::to_string(habit.id), 3) << "  ";
        if (done) {
            std::cout << "[x]    ";
        } else if (habit.frozen.count(today) > 0) {
            std::cout << "[*]    ";
        } else {
            std::cout << "[ ]    ";
        }
        std::cout << padRight(habit.name, width) << "  ";
        std::cout << padLeft(std::to_string(freezesLeft(habit, today)), 7) << "  ";
        std::cout << daysWord(currentStreak(habit, today));
        if (streakInDanger(habit, today)) {
            std::cout << "  !";
            danger = true;
        }
        std::cout << "\n";
    }

    std::cout << "\nDone today: " << doneToday << " of " << habits.size() << "\n";
    if (danger) {
        std::cout << "! means the streak will be lost if you skip today (or use: habits freeze ID)\n";
    }
}

void printLog(const std::vector<Habit>& habits, int today, int days) {
    if (habits.empty()) {
        std::cout << "No habits yet\n";
        return;
    }

    size_t width = nameColumnWidth(habits);
    int start = today - days + 1;

    std::cout << "     " << padRight("", width) << " ";
    for (int day = start; day <= today; day++) {
        std::cout << " " << weekdayName(day);
    }
    std::cout << "\n";

    std::cout << " ID  " << padRight("Habit", width) << " ";
    for (int day = start; day <= today; day++) {
        std::cout << " " << padLeft(std::to_string(daysToDate(day).day), 2);
    }
    std::cout << "\n";

    for (const Habit& habit : habits) {
        std::cout << padLeft(std::to_string(habit.id), 3) << "  " << padRight(habit.name, width) << " ";
        for (int day = start; day <= today; day++) {
            if (habit.done.count(day) > 0) {
                std::cout << "  x";
            } else if (habit.frozen.count(day) > 0) {
                std::cout << "  *";
            } else if (day < firstDay(habit)) {
                std::cout << "   ";
            } else {
                std::cout << "  .";
            }
        }
        std::cout << "\n";
    }

    std::cout << "\nx - done, * - frozen, . - missed, from " << formatDays(start) << " to " << formatDays(today) << "\n";
}

void printStats(const std::vector<Habit>& habits, int today) {
    if (habits.empty()) {
        std::cout << "No habits yet\n";
        return;
    }

    size_t width = nameColumnWidth(habits);

    std::cout << " ID  " << padRight("Habit", width) << "  Since       Done  Days  Percent  Streak  Best  Frozen  Freezes\n";
    for (const Habit& habit : habits) {
        std::cout << padLeft(std::to_string(habit.id), 3) << "  ";
        std::cout << padRight(habit.name, width) << "  ";
        std::cout << formatDays(firstDay(habit)) << "  ";
        std::cout << padLeft(std::to_string(doneCount(habit, today)), 4) << "  ";
        std::cout << padLeft(std::to_string(totalDays(habit, today)), 4) << "  ";
        std::cout << padLeft(std::to_string(percentDone(habit, today)) + "%", 7) << "  ";
        std::cout << padLeft(std::to_string(currentStreak(habit, today)), 6) << "  ";
        std::cout << padLeft(std::to_string(bestStreak(habit)), 4) << "  ";
        std::cout << padLeft(std::to_string(habit.frozen.size()), 6) << "  ";
        std::cout << padLeft(std::to_string(freezesLeft(habit, today)), 7) << "\n";
    }
}

static bool askLine(const std::string& question, std::string& answer) {
    std::cout << question;
    if (!std::getline(std::cin, answer)) {
        return false;
    }
    if (!answer.empty() && answer.back() == '\r') {
        answer.pop_back();
    }
    return true;
}

static bool askId(int& id) {
    std::string text;
    if (!askLine("Habit id: ", text)) {
        return false;
    }
    if (!parseNumber(text, id)) {
        std::cout << "This is not a number\n";
        return false;
    }
    return true;
}

void runMenu(std::vector<Habit>& habits, const std::string& dataPath) {
    while (true) {
        int today = todayDays();

        std::cout << "\n";
        std::cout << "1. Show habits\n";
        std::cout << "2. Add habit\n";
        std::cout << "3. Mark done today\n";
        std::cout << "4. Remove mark for today\n";
        std::cout << "5. Freeze a day\n";
        std::cout << "6. Log for 14 days\n";
        std::cout << "7. Statistics\n";
        std::cout << "8. Export to CSV\n";
        std::cout << "9. Remove habit\n";
        std::cout << "0. Exit\n";

        std::string choice;
        if (!askLine("> ", choice) || choice == "0") {
            break;
        }
        std::cout << "\n";

        bool changed = false;

        if (choice == "1") {
            printList(habits, today);
        } else if (choice == "2") {
            std::string name;
            if (askLine("Habit name: ", name)) {
                changed = addHabit(habits, name, today);
            }
        } else if (choice == "3") {
            int id = 0;
            printList(habits, today);
            if (askId(id)) {
                changed = markDone(habits, id, today, today);
            }
        } else if (choice == "4") {
            int id = 0;
            printList(habits, today);
            if (askId(id)) {
                changed = unmarkDone(habits, id, today);
            }
        } else if (choice == "5") {
            int id = 0;
            printList(habits, today);
            if (askId(id)) {
                std::string text;
                int day = today;
                if (askLine("Date (today, yesterday or YYYY-MM-DD): ", text)) {
                    if (parseDay(text, today, day)) {
                        changed = freezeDay(habits, id, day, today);
                    } else {
                        std::cout << "Bad date\n";
                    }
                }
            }
        } else if (choice == "6") {
            printLog(habits, today, 14);
        } else if (choice == "7") {
            printStats(habits, today);
        } else if (choice == "8") {
            std::string path;
            if (askLine("File name (for example habits.csv): ", path) && !path.empty()) {
                if (exportCsv(path, habits, today)) {
                    std::cout << "Saved to " << path << "\n";
                } else {
                    std::cout << "Cannot write file " << path << "\n";
                }
            }
        } else if (choice == "9") {
            int id = 0;
            printList(habits, today);
            if (askId(id)) {
                changed = removeHabit(habits, id);
            }
        } else {
            std::cout << "Unknown menu item\n";
        }

        if (changed && !saveHabits(dataPath, habits)) {
            std::cout << "Cannot save file " << dataPath << "\n";
        }
    }
}
