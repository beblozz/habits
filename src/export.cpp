#include "export.h"

#include <fstream>

#include "date.h"
#include "stats.h"

std::string csvField(const std::string& text) {
    bool needQuotes = false;
    for (char c : text) {
        if (c == ',' || c == '"' || c == '\n' || c == '\r') {
            needQuotes = true;
        }
    }
    if (!needQuotes) {
        return text;
    }

    std::string result = "\"";
    for (char c : text) {
        if (c == '"') {
            result += "\"\"";
        } else {
            result += c;
        }
    }
    result += "\"";
    return result;
}

std::string makeCsv(const std::vector<Habit>& habits, int today) {
    std::string csv = "habit_id,habit,date,weekday,done,frozen\n";
    for (const Habit& habit : habits) {
        for (int day = firstDay(habit); day <= today; day++) {
            csv += std::to_string(habit.id) + ",";
            csv += csvField(habit.name) + ",";
            csv += formatDays(day) + ",";
            csv += weekdayName(day) + ",";
            csv += habit.done.count(day) > 0 ? "1," : "0,";
            csv += habit.frozen.count(day) > 0 ? "1" : "0";
            csv += "\n";
        }
    }
    return csv;
}

bool exportCsv(const std::string& path, const std::vector<Habit>& habits, int today) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    file << makeCsv(habits, today);
    file.close();
    return !file.fail();
}
