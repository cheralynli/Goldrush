#include "timer_display.h"

#include "ui.h"
#include "ui_helpers.h"

#include <algorithm>
#include <iomanip>
#include <sstream>


//Input: int remainingSeconds
//Output: string of time remaining
//Purpose: to display to the user the amount of time remaining in minigame
//Relation: used in memory and minesweeper minigames
std::string countdownTimerText(int remainingSeconds) {
    return "Time Remaining: " + std::to_string(std::max(0, remainingSeconds));
}

//Input: double remainingSeconds
//Output: string of time remaining
//Purpose: to display to the user the amount of time remaining in minigame
//Relation:
std::string countdownTimerText(double remainingSeconds) {
    std::ostringstream out;
    out << "Time Remaining: " << std::fixed << std::setprecision(1)
        << std::max(0.0, remainingSeconds) << "s";
    return out.str();
}

//Input: WINDOW win, int y(rows), int x(columns), int remainingSeconds, bool hasColor
//Output: none
//Purpose: draws countdown timer with optional coloring
//Relation: used in memory and minesweeper minigames
void drawCountdownTimer(WINDOW* win, int y, int x, int remainingSeconds, bool hasColor) {
    if (!win) {
        return;
    }
    if (hasColor) {
        wattron(win, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);
    } else {
        wattron(win, A_BOLD);
    }
    mvwprintw(win, y, x, "%s", countdownTimerText(remainingSeconds).c_str());
    if (hasColor) {
        wattroff(win, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);
    } else {
        wattroff(win, A_BOLD);
    }
}

//Input: WINDOW win, int y(rows), int x(columns), double remainingSeconds, bool hasColor
//Output: none
//Purpose: draws countdown timer with optional coloring
//Relation:
void drawCountdownTimer(WINDOW* win, int y, int x, double remainingSeconds, bool hasColor) {
    if (!win) {
        return;
    }
    if (hasColor) {
        wattron(win, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);
    } else {
        wattron(win, A_BOLD);
    }
    mvwprintw(win, y, x, "%s", countdownTimerText(remainingSeconds).c_str());
    if (hasColor) {
        wattroff(win, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);
    } else {
        wattroff(win, A_BOLD);
    }
}

//Input: int seconds, bool hasColor
//Output: none
//Purpose: displays and counts down the timer
//Relation: used in memory and minesweeper minigames
void displayCountdownTimer(int seconds, bool hasColor) {
    int popupH = 7;
    int popupW = 42;
    WINDOW* popup = createCenteredWindow(popupH, popupW, 5, 24);
    if (!popup) {
        showTerminalSizeWarning(5, 24, hasColor);
        return;
    }
    getmaxyx(popup, popupH, popupW);
    const int contentW = std::max(1, popupW - 4);

    for (int remaining = seconds; remaining >= 0; --remaining) {
        werase(popup);
        drawBoxSafe(popup);
        mvwprintw(popup, 1, 2, "%s",
                  clipUiText("Countdown Timer Test", static_cast<std::size_t>(contentW)).c_str());
        drawCountdownTimer(popup, 3, 2, remaining, hasColor);
        wrefresh(popup);
        napms(remaining == 0 ? 700 : 1000);
    }

    delwin(popup);
    touchwin(stdscr);
    refresh();
}
