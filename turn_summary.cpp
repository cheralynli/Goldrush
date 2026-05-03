#include "turn_summary.h"

#include <algorithm>
#include <cstdlib>

#include <ncurses.h>

#include "ui.h"
#include "ui_helpers.h"

namespace {
//Input: string label, string value, int colorPair
//Output: none
//Purpose: structure for each line in summary
//Relation: used to complete summary with important information
struct SummaryLine {
    std::string label;
    std::string value;
    int colorPair;
};

//Input: WINDOW win, int y(rows), string text, int attrs
//Output: none
//Purpose: draws text horizontally and centered
//Relation: used by turn_summary UI functions
void centerPrint(WINDOW* win, int y, const std::string& text, int attrs = A_NORMAL) {
    int h = 0;
    int w = 0;
    getmaxyx(win, h, w);
    (void)h;
    const std::string clipped = clipUiText(text, static_cast<std::size_t>(std::max(1, w - 2)));
    if (attrs != A_NORMAL) {
        wattron(win, attrs);
    }
    mvwprintw(win, y, std::max(1, (w - static_cast<int>(clipped.size())) / 2), "%s", clipped.c_str());
    if (attrs != A_NORMAL) {
        wattroff(win, attrs);
    }
}

void drawSummaryArt(WINDOW* win, int startY, bool hasColor) {
    static const char* const SUMMARY_ART[] = {
        "   _____ _    _ __  __ __  __          _______     __",
        "  / ____| |  | |  \\/  |  \\/  |   /\\   |  __ \\ \\   / /",
        " | (___ | |  | | \\  / | \\  / |  /  \\  | |__) \\ \\_/ / ",
        "  \\___ \\| |  | | |\\/| | |\\/| | / /\\ \\ |  _  / \\   /  ",
        "  ____) | |__| | |  | | |  | |/ ____ \\| | \\ \\  | |   ",
        " |_____/ \\____/|_|  |_|_|  |_/_/    \\_\\_|  \\_\\ |_|   ",
        "                                                     ",
        "                                                     "
    };
    const int artLines = static_cast<int>(sizeof(SUMMARY_ART) / sizeof(SUMMARY_ART[0]));
    int w = 0;
    int h = 0;
    getmaxyx(win, h, w);
    (void)h;
    int artWidth = 0;
    for (int i = 0; i < artLines; ++i) {
        artWidth = std::max(artWidth, static_cast<int>(std::strlen(SUMMARY_ART[i])));
    }
    const int startX = std::max(2, (w - artWidth) / 2);

    if (hasColor) {
        wattron(win, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);
    } else {
        wattron(win, A_BOLD);
    }
    for (int i = 0; i < artLines; ++i) {
        mvwprintw(win, startY + i, startX, "%s", SUMMARY_ART[i]);
    }
    if (hasColor) {
        wattroff(win, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);
    } else {
        wattroff(win, A_BOLD);
    }
}

//Input: vector lines, amount, string gainLabel, string lossLabel, string unit, int gainColor, int lossColor
//Output: none
//Purpose: add line to summary regarding unit changes, when change amount != 0
//Relation: during a turn when a unit is changed, a line is added to the summary to inform the user
void addChange(std::vector<SummaryLine>& lines,
               int amount,
               const std::string& gainLabel,
               const std::string& lossLabel,
               const std::string& unit,
               int gainColor,
               int lossColor) {
    if (amount == 0) {
        return;
    }
    lines.push_back({
        amount > 0 ? gainLabel : lossLabel,
        unit == "cash" ? formatMoney(amount) : formatSignedChange(amount, unit),
        amount > 0 ? gainColor : lossColor
    });
}

//Input: vector lines, string label, string value, int colorPair
//Output: none
//Purpose: add additional text regarding important events
//Relation: during a turn when events occur, a line is added to the summary to inform the user
void addText(std::vector<SummaryLine>& lines,
             const std::string& label,
             const std::string& value,
             int colorPair) {
    if (!value.empty()) {
        lines.push_back({label, value, colorPair});
    }
}

//Input: WINDOW win, int y(rows), SummaryLine line, int width
//Output: none
//Purpose: draws summary section with important information
//Relation: called by showTurnSummaryReport
void drawSummarySection(WINDOW* win, int& y, const SummaryLine& line, int width) {
    if (y + 1 >= getmaxy(win) - 2) {
        return;
    }
    wattron(win, COLOR_PAIR(line.colorPair) | A_BOLD);
    mvwprintw(win, y, 3, "%s", clipUiText(line.label, static_cast<std::size_t>(std::max(8, width - 6))).c_str());
    wattroff(win, COLOR_PAIR(line.colorPair) | A_BOLD);
    mvwprintw(win,
              y + 1,
              5,
              "%s",
              clipUiText(line.value, static_cast<std::size_t>(std::max(8, width - 10))).c_str());
    y += 3;
}
}

//Input: int amount
//Output: string formatted to +-$amount
//Purpose: to format amount for display
//Relation: used when user money/investment changes
std::string formatMoney(int amount) {
    const int absolute = std::abs(amount);
    return std::string(amount < 0 ? "-$" : "$") + std::to_string(absolute);
}

//Input: int amount, string unit (item being changed)
//Output: string formatted to +-amount unit
//Purpose: to format amount for display
//Relation: used when user property changes
std::string formatSignedChange(int amount, const std::string& unit) {
    const std::string sign = amount > 0 ? "+" : "-";
    return sign + std::to_string(std::abs(amount)) + (unit.empty() ? "" : " " + unit);
}

//Input: TurnSummary summary, bool hasColor
//Output: none
//Purpose: build Summary popup at end of turn
//Relation: called during end of turn to inform user on important events and changes
void showTurnSummaryReport(const TurnSummary& summary, bool hasColor) {
    std::vector<SummaryLine> lines;
    addChange(lines, summary.moneyChange, "MONEY GAIN", "MONEY LOSS", "cash", GOLDRUSH_BLACK_FOREST, GOLDRUSH_GOLD_TERRA);
    addChange(lines, summary.loanChange, "LOAN GAIN", "LOAN LOSS", "loan", GOLDRUSH_GOLD_TERRA, GOLDRUSH_BLACK_FOREST);
    addChange(lines, summary.babyChange, "BABY GAIN", "BABY LOSS", "baby", GOLDRUSH_PLAYER_TWO, GOLDRUSH_GOLD_TERRA);
    addChange(lines, summary.petChange, "PET GAIN", "PET LOSS", "pet", GOLDRUSH_TILE_MINIGAME, GOLDRUSH_GOLD_TERRA);
    addChange(lines, summary.investmentChange, "INVESTMENT GAIN", "INVESTMENT LOSS", "cash", GOLDRUSH_BLACK_FOREST, GOLDRUSH_GOLD_TERRA);
    addChange(lines, summary.shieldChange, "SHIELD GAIN", "SHIELD LOSS", "shield", GOLDRUSH_GOLD_SAND, GOLDRUSH_GOLD_TERRA);
    addChange(lines, summary.insuranceChange, "INSURANCE GAIN", "INSURANCE LOSS", "use", GOLDRUSH_GOLD_SAND, GOLDRUSH_GOLD_TERRA);

    if (summary.gotMarried) {
        addText(lines, "MARRIAGE EVENT", summary.playerName + " got married and begins a new chapter.", GOLDRUSH_GOLD_SAND);
    }
    if (summary.jobChanged) {
        addText(lines, "JOB CHANGE", summary.oldJob + " -> " + summary.newJob, GOLDRUSH_TILE_CAREER);
    }
    if (summary.houseChanged) {
        addText(lines, "HOUSE CHANGE", summary.oldHouse.empty() ? summary.newHouse : summary.oldHouse + " -> " + summary.newHouse, GOLDRUSH_TILE_HOME);
    }
    for (std::size_t i = 0; i < summary.cardsGained.size(); ++i) {
        addText(lines, "CARD GAIN", summary.cardsGained[i], GOLDRUSH_TILE_ACTION);
    }
    for (std::size_t i = 0; i < summary.cardsUsed.size(); ++i) {
        addText(lines, "CARD USED", summary.cardsUsed[i], GOLDRUSH_TILE_ACTION);
    }
    for (std::size_t i = 0; i < summary.minigameResults.size(); ++i) {
        addText(lines, "MINIGAME RESULT", summary.minigameResults[i], GOLDRUSH_TILE_MINIGAME);
    }
    for (std::size_t i = 0; i < summary.sabotageEvents.size(); ++i) {
        addText(lines, "SABOTAGE EVENT", summary.sabotageEvents[i], GOLDRUSH_GOLD_TERRA);
    }
    for (std::size_t i = 0; i < summary.importantEvents.size(); ++i) {
        addText(lines, "IMPORTANT EVENT", summary.importantEvents[i], GOLDRUSH_GOLD_SAND);
    }
    if (lines.empty()) {
        addText(lines, "IMPORTANT EVENT", "No tracked stat changes this turn.", GOLDRUSH_BROWN_CREAM);
    }

    while (true) {
        int screenH = 0;
        int screenW = 0;
        getmaxyx(stdscr, screenH, screenW);
        const UILayout layout = calculateUILayout(screenH, screenW);
        const int boardInnerW = std::max(48, layout.boardWidth - 4);
        const int boardInnerH = std::max(14, layout.boardHeight - 4);
        const int minimumSummaryWidth = 58;
        const int desiredW = std::min(76, std::max(minimumSummaryWidth, boardInnerW));
        const int desiredH = std::min(std::max(18, 8 + static_cast<int>(lines.size()) * 3), boardInnerH);
        const int popupW = std::max(48, std::min(desiredW, boardInnerW));
        const int popupH = std::max(14, std::min(desiredH, boardInnerH));
        const int popupY = layout.originY + layout.headerHeight + 1 + std::max(0, (boardInnerH - popupH) / 2);
        const int popupX = layout.originX + 1 + std::max(0, (boardInnerW - popupW) / 2);

        if (popupY < 0 || popupX < 0 || popupY + popupH > screenH || popupX + popupW > screenW) {
            showTerminalSizeWarning(14, 48, hasColor);
            continue;
        }
        WINDOW* popup = newwin(popupH, popupW, popupY, popupX);
        if (!popup) {
            showTerminalSizeWarning(14, 48, hasColor);
            continue;
        }
        apply_ui_background(popup);
        keypad(popup, TRUE);

        int actualH = 0;
        int actualW = 0;
        getmaxyx(popup, actualH, actualW);
        werase(popup);
        drawBoxSafe(popup);

        drawSummaryArt(popup, 1, hasColor);
        centerPrint(popup,
                    8,
                    summary.playerName + " End-of-Turn " + std::to_string(summary.turnNumber) + " Summary",
                    A_BOLD);

        int y = 10;
        for (std::size_t i = 0; i < lines.size(); ++i) {
            drawSummarySection(popup, y, lines[i], actualW);
        }

        mvwprintw(popup, actualH - 2, 3, "%s",
                  clipUiText("Press ENTER or ESC to continue...", static_cast<std::size_t>(std::max(1, actualW - 5))).c_str());
        wrefresh(popup);

        const int ch = wgetch(popup);
        delwin(popup);
        if (ch == KEY_RESIZE) {
            continue;
        }
        if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
            break;
        }
    }
}
