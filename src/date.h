#pragma once

#include <string>

struct Date {
    int year = 1970;
    int month = 1;
    int day = 1;
};

bool isLeapYear(int year);
int daysInMonth(int year, int month);
bool isValidDate(const Date& date);
int dateToDays(const Date& date);
Date daysToDate(int days);
int weekdayIndex(int days);
std::string weekdayName(int days);
bool parseDate(const std::string& text, Date& date);
std::string formatDate(const Date& date);
std::string formatDays(int days);
int todayDays();
