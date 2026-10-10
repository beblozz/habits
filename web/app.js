const e = React.createElement;

const WEEKDAYS = ["Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"];
const WEEKDAY_NAMES = ["Пн", "Вт", "Ср", "Чт", "Пт", "Сб", "Вс"];
const MAX_DAYS = 140;
const DAYS_FOR_FREEZE = 7;

function parseCsvLine(line) {
    const fields = [];
    let field = "";
    let inQuotes = false;

    for (let i = 0; i < line.length; i++) {
        const c = line[i];
        if (inQuotes) {
            if (c === '"' && line[i + 1] === '"') {
                field += '"';
                i++;
            } else if (c === '"') {
                inQuotes = false;
            } else {
                field += c;
            }
        } else if (c === '"') {
            inQuotes = true;
        } else if (c === ",") {
            fields.push(field);
            field = "";
        } else {
            field += c;
        }
    }
    fields.push(field);
    return fields;
}

function parseCsv(text) {
    const lines = text.split("\n");
    const habits = [];

    for (let i = 1; i < lines.length; i++) {
        const line = lines[i].replace("\r", "");
        if (line === "") {
            continue;
        }
        const fields = parseCsvLine(line);
        if (fields.length !== 5 && fields.length !== 6) {
            return null;
        }

        let habit = habits.find(function (h) { return h.id === fields[0]; });
        if (!habit) {
            habit = { id: fields[0], name: fields[1], days: [] };
            habits.push(habit);
        }
        habit.days.push({
            date: fields[2],
            weekday: fields[3],
            done: fields[4] === "1",
            frozen: fields[5] === "1"
        });
    }
    return habits;
}

function currentStreak(days) {
    let i = days.length - 1;
    if (i >= 0 && !days[i].done && !days[i].frozen) {
        i--;
    }
    let streak = 0;
    while (i >= 0 && (days[i].done || days[i].frozen)) {
        if (days[i].done) {
            streak++;
        }
        i--;
    }
    return streak;
}

function bestStreak(days) {
    let best = 0;
    let current = 0;
    for (let i = 0; i < days.length; i++) {
        if (days[i].done) {
            current++;
        } else if (!days[i].frozen) {
            current = 0;
        }
        if (current > best) {
            best = current;
        }
    }
    return best;
}

function doneCount(days) {
    let count = 0;
    for (let i = 0; i < days.length; i++) {
        if (days[i].done) {
            count++;
        }
    }
    return count;
}

function frozenCount(days) {
    let count = 0;
    for (let i = 0; i < days.length; i++) {
        if (days[i].frozen) {
            count++;
        }
    }
    return count;
}

function freezesLeft(days) {
    const left = Math.floor(doneCount(days) / DAYS_FOR_FREEZE) - frozenCount(days);
    if (left < 0) {
        return 0;
    }
    return left;
}

function anyFrozen(habits, back) {
    for (let i = 0; i < habits.length; i++) {
        const days = habits[i].days;
        const index = days.length - 1 - back;
        if (index >= 0 && days[index].frozen) {
            return true;
        }
    }
    return false;
}

function anyDone(habits, back) {
    for (let i = 0; i < habits.length; i++) {
        const days = habits[i].days;
        const index = days.length - 1 - back;
        if (index >= 0 && days[index].done) {
            return true;
        }
    }
    return false;
}

function longestDays(habits) {
    let longest = habits[0].days;
    for (let i = 1; i < habits.length; i++) {
        if (habits[i].days.length > longest.length) {
            longest = habits[i].days;
        }
    }
    return longest;
}

function totalStreak(habits) {
    let back = 0;
    if (!anyDone(habits, 0) && !anyFrozen(habits, 0)) {
        back = 1;
    }
    let streak = 0;
    while (anyDone(habits, back) || anyFrozen(habits, back)) {
        if (anyDone(habits, back)) {
            streak++;
        }
        back++;
    }
    return streak;
}

function flameState(streak, doneToday, frozenToday) {
    if (streak === 0) {
        return "off";
    }
    if (doneToday) {
        return "on";
    }
    if (frozenToday) {
        return "frozen";
    }
    return "risk";
}

function daysWord(count) {
    const lastTwo = count % 100;
    const last = count % 10;
    if (lastTwo >= 11 && lastTwo <= 14) {
        return "дней";
    }
    if (last === 1) {
        return "день";
    }
    if (last >= 2 && last <= 4) {
        return "дня";
    }
    return "дней";
}

function todayText() {
    const now = new Date();
    const month = now.getMonth() + 1;
    const day = now.getDate();
    return now.getFullYear() + "-" + (month < 10 ? "0" : "") + month + "-" + (day < 10 ? "0" : "") + day;
}

function Flame(props) {
    return e(
        "svg",
        {
            className: "flame " + props.state,
            width: props.size,
            height: props.size,
            viewBox: "0 0 24 24"
        },
        e("path", {
            className: "flame-outer",
            d: "M12 2C12 6 6 8.5 6 14.5C6 18.6 8.7 22 12 22C15.3 22 18 18.6 18 14.5C18 12 17 10.2 16 9C15.6 10.6 14.8 11.4 14 11.6C14.6 8 13.6 4.2 12 2Z"
        }),
        e("path", {
            className: "flame-inner",
            d: "M12 12.5C10.5 14.3 9.5 15.6 9.5 17.3C9.5 18.9 10.6 20 12 20C13.4 20 14.5 18.9 14.5 17.3C14.5 15.6 13.5 14.3 12 12.5Z"
        })
    );
}

function Snowflake(props) {
    return e(
        "svg",
        { className: "snowflake", width: props.size, height: props.size, viewBox: "0 0 24 24" },
        e("path", { d: "M12 3V21M4.2 7.5L19.8 16.5M19.8 7.5L4.2 16.5" }),
        e("path", { d: "M9.5 4.5L12 7L14.5 4.5M9.5 19.5L12 17L14.5 19.5" })
    );
}

function WeekRow(props) {
    const days = longestDays(props.habits);
    const items = [];

    for (let back = 6; back >= 0; back--) {
        const index = days.length - 1 - back;
        if (index < 0) {
            continue;
        }
        const done = anyDone(props.habits, back);
        const frozen = !done && anyFrozen(props.habits, back);
        let className = "week-day";
        let icon = e(Flame, { size: 18, state: done ? "on" : "off" });
        if (done) {
            className += " done";
        } else if (frozen) {
            className += " frozen";
            icon = e(Snowflake, { size: 18 });
        } else if (back === 0) {
            className += " today";
        }
        items.push(e(
            "div",
            { className: className, key: days[index].date, title: days[index].date },
            e("span", { className: "week-name" }, WEEKDAY_NAMES[WEEKDAYS.indexOf(days[index].weekday)]),
            e("span", { className: "week-circle" }, icon)
        ));
    }

    return e("div", { className: "week" }, items);
}

function StreakPanel(props) {
    const streak = totalStreak(props.habits);
    const doneToday = anyDone(props.habits, 0);
    const state = flameState(streak, doneToday, anyFrozen(props.habits, 0));

    let text = "Отметь любую привычку, чтобы зажечь огонёк";
    if (state === "on") {
        text = "Сегодня серия продлена. Так держать!";
    } else if (state === "risk") {
        text = "Отметь хотя бы одну привычку сегодня, чтобы не потерять серию";
    } else if (state === "frozen") {
        text = "Сегодня день заморожен, серия сохранена";
    }

    return e(
        "section",
        { className: "streak " + state },
        e(Flame, { size: 96, state: state }),
        e(
            "div",
            { className: "streak-info" },
            e(
                "div",
                { className: "streak-count" },
                e("span", { className: "streak-number" }, streak),
                " " + daysWord(streak) + " подряд"
            ),
            e("p", { className: "streak-text" }, text),
            e(WeekRow, { habits: props.habits })
        )
    );
}

function Stat(props) {
    return e(
        "div",
        { className: "stat" },
        e("div", { className: "stat-value" }, props.value),
        e("div", { className: "stat-label" }, props.label)
    );
}

function Heatmap(props) {
    let days = props.days;
    if (days.length > MAX_DAYS) {
        days = days.slice(days.length - MAX_DAYS);
    }

    const cells = [];
    const offset = WEEKDAYS.indexOf(days[0].weekday);
    for (let i = 0; i < offset; i++) {
        cells.push(e("div", { className: "cell hidden", key: "empty" + i }));
    }
    for (let i = 0; i < days.length; i++) {
        const day = days[i];
        let className = "cell";
        let title = day.date + " пропущено";
        if (day.done) {
            className = "cell done";
            title = day.date + " сделано";
        } else if (day.frozen) {
            className = "cell frozen";
            title = day.date + " заморожено";
        }
        cells.push(e("div", { className: className, key: day.date, title: title }));
    }

    return e(
        "div",
        { className: "heatmap" },
        e(
            "div",
            { className: "weekdays" },
            e("span", null, "Пн"),
            e("span", null, "Ср"),
            e("span", null, "Пт"),
            e("span", null, "Вс")
        ),
        e("div", { className: "cells" }, cells)
    );
}

function HabitCard(props) {
    const days = props.habit.days;
    const done = doneCount(days);
    const percent = Math.round(done * 100 / days.length);
    const doneToday = days[days.length - 1].done;
    const frozenToday = days[days.length - 1].frozen;
    const streak = currentStreak(days);
    const state = flameState(streak, doneToday, frozenToday);

    let badgeClass = "badge";
    let badgeText = "сегодня нет";
    if (doneToday) {
        badgeClass = "badge done";
        badgeText = "сегодня сделано";
    } else if (frozenToday) {
        badgeClass = "badge frozen";
        badgeText = "сегодня заморожено";
    }

    return e(
        "section",
        { className: "card" },
        e(
            "div",
            { className: "card-header" },
            e(
                "div",
                { className: "card-title" },
                e(
                    "span",
                    { className: "flame-chip " + state },
                    e(Flame, { size: 22, state: state }),
                    streak
                ),
                e("h2", null, props.habit.name)
            ),
            e("span", { className: badgeClass }, badgeText)
        ),
        e(
            "div",
            { className: "stats" },
            e(Stat, { value: streak, label: "текущая серия" }),
            e(Stat, { value: bestStreak(days), label: "лучшая серия" }),
            e(Stat, { value: done + " из " + days.length, label: "дней сделано" }),
            e(Stat, { value: percent + "%", label: "выполнение" })
        ),
        e(Heatmap, { days: days }),
        e(
            "div",
            { className: "legend" },
            e("span", { className: "cell done" }),
            "сделано",
            e("span", { className: "cell frozen" }),
            "заморожено",
            e("span", { className: "cell" }),
            "пропущено"
        ),
        e(
            "p",
            { className: "range" },
            "с " + days[0].date + " по " + days[days.length - 1].date +
            ", заморозок в запасе: " + freezesLeft(days) + ", использовано: " + frozenCount(days)
        )
    );
}

function App() {
    const [habits, setHabits] = React.useState([]);
    const [fileName, setFileName] = React.useState("");
    const [error, setError] = React.useState("");

    function handleFile(event) {
        const file = event.target.files[0];
        if (!file) {
            return;
        }
        const reader = new FileReader();
        reader.onload = function () {
            const result = parseCsv(reader.result);
            if (result === null || result.length === 0) {
                setHabits([]);
                setError("Не получилось прочитать файл. Нужен CSV из команды habits export");
            } else {
                setHabits(result);
                setError("");
            }
            setFileName(file.name);
        };
        reader.readAsText(file);
    }

    let content;
    if (error !== "") {
        content = e("p", { className: "message error" }, error);
    } else if (habits.length === 0) {
        content = e("p", { className: "message" }, "Сделай файл командой habits export habits.csv и открой его здесь");
    } else {
        const lastDays = habits[0].days;
        const lastDate = lastDays[lastDays.length - 1].date;
        let note = null;
        if (lastDate !== todayText()) {
            note = e("p", { className: "note" }, "Данные в файле на " + lastDate + ". Сделай habits export ещё раз, чтобы увидеть сегодняшний день");
        }
        content = e(
            "div",
            null,
            note,
            e(StreakPanel, { habits: habits }),
            habits.map(function (habit) {
                return e(HabitCard, { habit: habit, key: habit.id });
            })
        );
    }

    let summary = "";
    if (habits.length > 0) {
        let doneToday = 0;
        for (let i = 0; i < habits.length; i++) {
            const days = habits[i].days;
            if (days[days.length - 1].done) {
                doneToday++;
            }
        }
        const lastDays = habits[0].days;
        summary = lastDays[lastDays.length - 1].date + ": сделано " + doneToday + " из " + habits.length;
    }

    return e(
        "div",
        { className: "app" },
        e(
            "header",
            { className: "top" },
            e(
                "div",
                null,
                e("h1", null, "Habits viewer"),
                e("p", null, summary !== "" ? summary : "Календарь привычек по файлу из программы habits")
            ),
            e(
                "label",
                { className: "file-button" },
                fileName !== "" ? fileName : "Открыть .csv",
                e("input", { type: "file", accept: ".csv", onChange: handleFile })
            )
        ),
        content
    );
}

const root = ReactDOM.createRoot(document.getElementById("root"));
root.render(e(App));
