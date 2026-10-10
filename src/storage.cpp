#include "storage.h"

#include <filesystem>
#include <fstream>

#include "date.h"

static std::vector<std::string> splitLine(const std::string& line, size_t maxParts) {
    std::vector<std::string> parts;
    size_t start = 0;
    while (parts.size() + 1 < maxParts) {
        size_t pos = line.find('|', start);
        if (pos == std::string::npos) {
            break;
        }
        parts.push_back(line.substr(start, pos - start));
        start = pos + 1;
    }
    parts.push_back(line.substr(start));
    return parts;
}

static bool parseId(const std::string& text, int& id) {
    if (text.empty() || text.size() > 6) {
        return false;
    }
    for (char c : text) {
        if (c < '0' || c > '9') {
            return false;
        }
    }
    id = std::stoi(text);
    return id > 0;
}

int findHabit(const std::vector<Habit>& habits, int id) {
    for (size_t i = 0; i < habits.size(); i++) {
        if (habits[i].id == id) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int nextHabitId(const std::vector<Habit>& habits) {
    int maxId = 0;
    for (const Habit& habit : habits) {
        if (habit.id > maxId) {
            maxId = habit.id;
        }
    }
    return maxId + 1;
}

bool loadHabits(const std::string& path, std::vector<Habit>& habits, std::string& error) {
    habits.clear();

    if (!std::filesystem::exists(path)) {
        return true;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        error = "cannot open file " + path;
        return false;
    }

    std::string line;
    int lineNumber = 0;
    while (std::getline(file, line)) {
        lineNumber++;
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }

        std::string where = path + ", line " + std::to_string(lineNumber);

        if (line.rfind("habit|", 0) == 0) {
            std::vector<std::string> parts = splitLine(line, 4);
            Habit habit;
            Date created;
            if (parts.size() != 4 || !parseId(parts[1], habit.id) ||
                !parseDate(parts[2], created) || parts[3].empty()) {
                error = "bad habit in " + where;
                return false;
            }
            if (findHabit(habits, habit.id) != -1) {
                error = "habit id is used twice in " + where;
                return false;
            }
            habit.created = dateToDays(created);
            habit.name = parts[3];
            habits.push_back(habit);
        } else if (line.rfind("done|", 0) == 0) {
            std::vector<std::string> parts = splitLine(line, 3);
            int id = 0;
            Date date;
            if (parts.size() != 3 || !parseId(parts[1], id) || !parseDate(parts[2], date)) {
                error = "bad mark in " + where;
                return false;
            }
            int index = findHabit(habits, id);
            if (index == -1) {
                error = "mark for unknown habit in " + where;
                return false;
            }
            habits[index].done.insert(dateToDays(date));
        } else if (line.rfind("freeze|", 0) == 0) {
            std::vector<std::string> parts = splitLine(line, 3);
            int id = 0;
            Date date;
            if (parts.size() != 3 || !parseId(parts[1], id) || !parseDate(parts[2], date)) {
                error = "bad freeze in " + where;
                return false;
            }
            int index = findHabit(habits, id);
            if (index == -1) {
                error = "freeze for unknown habit in " + where;
                return false;
            }
            habits[index].frozen.insert(dateToDays(date));
        } else {
            error = "unknown record in " + where;
            return false;
        }
    }

    for (Habit& habit : habits) {
        for (int day : habit.done) {
            habit.frozen.erase(day);
        }
    }

    return true;
}

bool saveHabits(const std::string& path, const std::vector<Habit>& habits) {
    std::string tempPath = path + ".tmp";

    std::ofstream file(tempPath);
    if (!file.is_open()) {
        return false;
    }

    for (const Habit& habit : habits) {
        file << "habit|" << habit.id << "|" << formatDays(habit.created) << "|" << habit.name << "\n";
    }
    for (const Habit& habit : habits) {
        for (int day : habit.done) {
            file << "done|" << habit.id << "|" << formatDays(day) << "\n";
        }
    }
    for (const Habit& habit : habits) {
        for (int day : habit.frozen) {
            file << "freeze|" << habit.id << "|" << formatDays(day) << "\n";
        }
    }

    file.close();
    if (file.fail()) {
        return false;
    }

    std::error_code code;
    std::filesystem::rename(tempPath, path, code);
    return !code;
}
