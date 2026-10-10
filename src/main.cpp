#include <iostream>
#include <string>
#include <vector>

#include "commands.h"
#include "date.h"
#include "export.h"
#include "storage.h"

void printUsage() {
    std::cout << "Usage: habits [-f FILE] COMMAND\n";
    std::cout << "\n";
    std::cout << "Commands:\n";
    std::cout << "  add NAME            add a new habit\n";
    std::cout << "  list                show habits and marks for today\n";
    std::cout << "  done ID [DATE]      mark habit as done\n";
    std::cout << "  undo ID [DATE]      remove the mark or the freeze\n";
    std::cout << "  freeze ID [DATE]    freeze a missed day to keep the streak\n";
    std::cout << "  rename ID NAME      change habit name\n";
    std::cout << "  remove ID           delete habit with all marks\n";
    std::cout << "  log [DAYS]          table for the last days (14 by default)\n";
    std::cout << "  stats               streaks and percent of done days\n";
    std::cout << "  export FILE         save all days to a CSV file\n";
    std::cout << "  menu                interactive menu\n";
    std::cout << "  help                show this help\n";
    std::cout << "\n";
    std::cout << "DATE is today, yesterday or a date like 2026-10-09\n";
    std::cout << "You get one freeze for every 7 done days\n";
    std::cout << "Data is stored in habits.txt, use -f FILE to change it\n";
}

std::string joinArgs(const std::vector<std::string>& args, size_t from) {
    std::string result;
    for (size_t i = from; i < args.size(); i++) {
        if (!result.empty()) {
            result += " ";
        }
        result += args[i];
    }
    return result;
}

int main(int argc, char* argv[]) {
    std::string dataPath = "habits.txt";
    std::vector<std::string> args;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-f" && args.empty()) {
            if (i + 1 >= argc) {
                std::cerr << "Option -f needs a file name\n";
                return 1;
            }
            dataPath = argv[i + 1];
            i++;
        } else {
            args.push_back(arg);
        }
    }

    if (args.empty()) {
        printUsage();
        return 1;
    }

    std::string command = args[0];
    if (command == "help" || command == "-h" || command == "--help") {
        printUsage();
        return 0;
    }

    std::vector<Habit> habits;
    std::string error;
    if (!loadHabits(dataPath, habits, error)) {
        std::cerr << "Error: " << error << "\n";
        return 1;
    }

    int today = todayDays();
    bool changed = false;

    if (command == "add") {
        if (args.size() < 2) {
            std::cerr << "Usage: habits add NAME\n";
            return 1;
        }
        if (!addHabit(habits, joinArgs(args, 1), today)) {
            return 1;
        }
        changed = true;
    } else if (command == "list") {
        printList(habits, today);
    } else if (command == "done" || command == "undo" || command == "freeze") {
        int id = 0;
        int day = today;
        if (args.size() < 2 || args.size() > 3 || !parseNumber(args[1], id)) {
            std::cerr << "Usage: habits " << command << " ID [DATE]\n";
            return 1;
        }
        if (args.size() == 3 && !parseDay(args[2], today, day)) {
            std::cerr << "Bad date \"" << args[2] << "\", use today, yesterday or YYYY-MM-DD\n";
            return 1;
        }
        bool ok = false;
        if (command == "done") {
            ok = markDone(habits, id, day, today);
        } else if (command == "freeze") {
            ok = freezeDay(habits, id, day, today);
        } else {
            ok = unmarkDone(habits, id, day);
        }
        if (!ok) {
            return 1;
        }
        changed = true;
    } else if (command == "rename") {
        int id = 0;
        if (args.size() < 3 || !parseNumber(args[1], id)) {
            std::cerr << "Usage: habits rename ID NAME\n";
            return 1;
        }
        if (!renameHabit(habits, id, joinArgs(args, 2))) {
            return 1;
        }
        changed = true;
    } else if (command == "remove") {
        int id = 0;
        if (args.size() != 2 || !parseNumber(args[1], id)) {
            std::cerr << "Usage: habits remove ID\n";
            return 1;
        }
        if (!removeHabit(habits, id)) {
            return 1;
        }
        changed = true;
    } else if (command == "log") {
        int days = 14;
        if (args.size() > 2 || (args.size() == 2 && !parseNumber(args[1], days)) || days < 1 || days > 60) {
            std::cerr << "Usage: habits log [DAYS], DAYS is from 1 to 60\n";
            return 1;
        }
        printLog(habits, today, days);
    } else if (command == "stats") {
        printStats(habits, today);
    } else if (command == "export") {
        if (args.size() != 2) {
            std::cerr << "Usage: habits export FILE\n";
            return 1;
        }
        if (!exportCsv(args[1], habits, today)) {
            std::cerr << "Error: cannot write file " << args[1] << "\n";
            return 1;
        }
        std::cout << "Saved to " << args[1] << "\n";
    } else if (command == "menu") {
        runMenu(habits, dataPath);
    } else {
        std::cerr << "Unknown command \"" << command << "\", try: habits help\n";
        return 1;
    }

    if (changed && !saveHabits(dataPath, habits)) {
        std::cerr << "Error: cannot save file " << dataPath << "\n";
        return 1;
    }

    return 0;
}
