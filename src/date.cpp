#include "date.h"

#include <cctype>
#include <ctime>

bool isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

int daysInMonth(int year, int month) {
    const int days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && isLeapYear(year)) {
        return 29;
    }
    return days[month - 1];
}

bool isValidDate(const Date& date) {
    if (date.year < 1970 || date.year > 9999) {
        return false;
    }
    if (date.month < 1 || date.month > 12) {
        return false;
    }
    if (date.day < 1 || date.day > daysInMonth(date.year, date.month)) {
        return false;
    }
    return true;
}

int dateToDays(const Date& date) {
    int days = 0;
    for (int year = 1970; year < date.year; year++) {
        days += isLeapYear(year) ? 366 : 365;
    }
    for (int month = 1; month < date.month; month++) {
        days += daysInMonth(date.year, month);
    }
    days += date.day - 1;
    return days;
}

Date daysToDate(int days) {
    Date date;
    date.year = 1970;
    while (days >= (isLeapYear(date.year) ? 366 : 365)) {
        days -= isLeapYear(date.year) ? 366 : 365;
        date.year++;
    }
    date.month = 1;
    while (days >= daysInMonth(date.year, date.month)) {
        days -= daysInMonth(date.year, date.month);
        date.month++;
    }
    date.day = days + 1;
    return date;
}

int weekdayIndex(int days) {
    return (days + 3) % 7;
}

std::string weekdayName(int days) {
    const std::string names[7] = {"Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"};
    return names[weekdayIndex(days)];
}

bool parseDate(const std::string& text, Date& date) {
    if (text.size() != 10 || text[4] != '-' || text[7] != '-') {
        return false;
    }
    for (size_t i = 0; i < text.size(); i++) {
        if (i == 4 || i == 7) {
            continue;
        }
        if (!std::isdigit(static_cast<unsigned char>(text[i]))) {
            return false;
        }
    }

    Date result;
    result.year = std::stoi(text.substr(0, 4));
    result.month = std::stoi(text.substr(5, 2));
    result.day = std::stoi(text.substr(8, 2));
    if (!isValidDate(result)) {
        return false;
    }

    date = result;
    return true;
}

static std::string twoDigits(int value) {
    if (value < 10) {
        return "0" + std::to_string(value);
    }
    return std::to_string(value);
}

std::string formatDate(const Date& date) {
    return std::to_string(date.year) + "-" + twoDigits(date.month) + "-" + twoDigits(date.day);
}

std::string formatDays(int days) {
    return formatDate(daysToDate(days));
}

int todayDays() {
    std::time_t now = std::time(nullptr);
    std::tm* local = std::localtime(&now);

    Date date;
    date.year = local->tm_year + 1900;
    date.month = local->tm_mon + 1;
    date.day = local->tm_mday;
    return dateToDays(date);
}
