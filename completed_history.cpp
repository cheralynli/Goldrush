#include "completed_history.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

#include <ncurses.h>

#include "input_helpers.h"
#include "ui.h"
#include "ui_helpers.h"

namespace fs = std::filesystem;

namespace {
fs::path historyPath() {
    return fs::path("saves") / "completed_history.log";
}

std::string valueAfter(const std::string& line, const std::string& prefix) {
    if (line.rfind(prefix, 0) != 0) {
        return "";
    }
    return line.substr(prefix.size());
}

bool parseHistoryInt(const std::string& text, int& value) {
    if (text.empty()) {
        return false;
    }
    char* endPtr = nullptr;
    const long parsed = std::strtol(text.c_str(), &endPtr, 10);
    if (endPtr == nullptr || *endPtr != '\0') {
        return false;
    }
    value = static_cast<int>(parsed);
    return true;
}

std::vector<std::string> fallbackDetailLines(const CompletedGameEntry& entry) {
    std::vector<std::string> lines;
    lines.push_back("Winner: " + entry.winner + " with $" + std::to_string(entry.winnerScore));
    lines.push_back("Game: " + entry.gameId);
    lines.push_back("Date: " + entry.date);
    lines.push_back("Mode: " + entry.mode);
    lines.push_back("Settings: " + (entry.settings.empty() ? "not recorded" : entry.settings));
    lines.push_back("Turns: " + std::to_string(entry.rounds));
    lines.push_back("");
    lines.push_back("Recorded players:");
    if (!entry.playerDetails.empty()) {
        lines.push_back(entry.playerDetails);
    } else if (!entry.players.empty()) {
        lines.push_back(entry.players);
    } else {
        lines.push_back("No player breakdown was recorded for this older entry.");
    }
    return lines;
}

void showCompletedGameDetailScreen(const CompletedGameEntry& entry, bool hasColor) {
    std::vector<std::string> lines = entry.detailLines.empty()
        ? fallbackDetailLines(entry)
        : entry.detailLines;
    if (lines.empty()) {
        lines.push_back("No details recorded for this completed game.");
    }

    int screenH = 0;
    int screenW = 0;
    getmaxyx(stdscr, screenH, screenW);
    int popupH = std::min(24, std::max(12, screenH - 2));
    int popupW = std::min(100, std::max(56, screenW - 2));
    WINDOW* popup = createCenteredWindow(popupH, popupW, 12, 56);
    if (!popup) {
        showTerminalSizeWarning(12, 56, hasColor);
        return;
    }
    keypad(popup, TRUE);
    getmaxyx(popup, popupH, popupW);
    const int contentW = std::max(1, popupW - 4);
    const int visible = std::max(1, popupH - 5);
    int top = 0;

    while (true) {
        if (top > std::max(0, static_cast<int>(lines.size()) - visible)) {
            top = std::max(0, static_cast<int>(lines.size()) - visible);
        }

        werase(popup);
        drawBoxSafe(popup);
        if (hasColor) {
            wattron(popup, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);
        }
        mvwprintw(popup, 1, 2, "%s",
                  clipUiText("COMPLETED GAME DETAILS", static_cast<std::size_t>(contentW)).c_str());
        if (hasColor) {
            wattroff(popup, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);
        }

        for (int row = 0; row < visible && top + row < static_cast<int>(lines.size()); ++row) {
            mvwprintw(popup,
                      3 + row,
                      2,
                      "%s",
                      clipUiText(lines[static_cast<std::size_t>(top + row)],
                                 static_cast<std::size_t>(contentW)).c_str());
        }

        mvwprintw(popup, popupH - 2, 2, "%s",
                  clipUiText("Up/Down scroll  ESC history",
                             static_cast<std::size_t>(contentW)).c_str());
        wrefresh(popup);

        const int ch = wgetch(popup);
        if (isCancelKey(ch)) {
            break;
        }
        if (ch == KEY_UP) {
            top = std::max(0, top - 1);
        } else if (ch == KEY_DOWN) {
            top = std::min(std::max(0, static_cast<int>(lines.size()) - visible), top + 1);
        } else if (ch == KEY_PPAGE) {
            top = std::max(0, top - visible);
        } else if (ch == KEY_NPAGE) {
            top = std::min(std::max(0, static_cast<int>(lines.size()) - visible), top + visible);
        }
    }

    delwin(popup);
    touchwin(stdscr);
    refresh();
}
}

bool appendCompletedGameHistory(const CompletedGameEntry& entry, std::string& error) {
    std::error_code ec;
    fs::create_directories(historyPath().parent_path(), ec);
    if (ec) {
        error = "Could not create saves directory.";
        return false;
    }

    std::ofstream out(historyPath().string().c_str(), std::ios::app);
    if (!out) {
        error = "Could not write completed history.";
        return false;
    }

    out << "GAME_COMPLETED\n";
    out << "date=" << entry.date << "\n";
    out << "gameId=" << entry.gameId << "\n";
    out << "winner=" << entry.winner << "\n";
    out << "winnerScore=" << entry.winnerScore << "\n";
    out << "winnerCash=" << entry.winnerCash << "\n";
    out << "winnerNetWorth=" << entry.winnerNetWorth << "\n";
    out << "rounds=" << entry.rounds << "\n";
    out << "players=" << entry.players << "\n";
    out << "playerDetails=" << entry.playerDetails << "\n";
    for (std::size_t i = 0; i < entry.detailLines.size(); ++i) {
        out << "detail=" << entry.detailLines[i] << "\n";
    }
    out << "mode=" << entry.mode << "\n";
    out << "settings=" << entry.settings << "\n";
    out << "END\n";
    return true;
}

std::vector<CompletedGameEntry> loadCompletedGameHistory(std::string& error) {
    std::vector<CompletedGameEntry> entries;
    std::ifstream in(historyPath().string().c_str());
    if (!in) {
        return entries;
    }

    std::string line;
    CompletedGameEntry current;
    bool reading = false;
    while (std::getline(in, line)) {
        if (line == "GAME_COMPLETED") {
            current = CompletedGameEntry();
            reading = true;
            continue;
        }
        if (line == "END") {
            if (reading) {
                entries.push_back(current);
            }
            reading = false;
            continue;
        }
        if (!reading) {
            continue;
        }
        if (line.rfind("date=", 0) == 0) current.date = valueAfter(line, "date=");
        else if (line.rfind("gameId=", 0) == 0) current.gameId = valueAfter(line, "gameId=");
        else if (line.rfind("winner=", 0) == 0) current.winner = valueAfter(line, "winner=");
        else if (line.rfind("winnerScore=", 0) == 0) {
            int parsed = current.winnerScore;
            if (parseHistoryInt(valueAfter(line, "winnerScore="), parsed)) current.winnerScore = parsed;
        }
        else if (line.rfind("winnerCash=", 0) == 0) {
            int parsed = current.winnerCash;
            if (parseHistoryInt(valueAfter(line, "winnerCash="), parsed)) current.winnerCash = parsed;
        }
        else if (line.rfind("winnerNetWorth=", 0) == 0) {
            int parsed = current.winnerNetWorth;
            if (parseHistoryInt(valueAfter(line, "winnerNetWorth="), parsed)) current.winnerNetWorth = parsed;
        }
        else if (line.rfind("rounds=", 0) == 0) {
            int parsed = current.rounds;
            if (parseHistoryInt(valueAfter(line, "rounds="), parsed)) current.rounds = parsed;
        }
        else if (line.rfind("players=", 0) == 0) current.players = valueAfter(line, "players=");
        else if (line.rfind("playerDetails=", 0) == 0) current.playerDetails = valueAfter(line, "playerDetails=");
        else if (line.rfind("detail=", 0) == 0) current.detailLines.push_back(valueAfter(line, "detail="));
        else if (line.rfind("mode=", 0) == 0) current.mode = valueAfter(line, "mode=");
        else if (line.rfind("settings=", 0) == 0) current.settings = valueAfter(line, "settings=");
    }

    if (!in.eof()) {
        error = "Could not read completed history.";
    }
    return entries;
}

void showCompletedGameHistoryScreen(bool hasColor) {
    std::string error;
    const std::vector<CompletedGameEntry> entries = loadCompletedGameHistory(error);
    int screenH = 0;
    int screenW = 0;
    getmaxyx(stdscr, screenH, screenW);
    int popupH = std::min(20, std::max(10, screenH - 2));
    int popupW = std::min(86, std::max(48, screenW - 2));
    WINDOW* popup = createCenteredWindow(popupH, popupW, 10, 48);
    if (!popup) {
        showTerminalSizeWarning(10, 48, hasColor);
        return;
    }
    keypad(popup, TRUE);
    getmaxyx(popup, popupH, popupW);
    const int contentW = std::max(1, popupW - 4);

    int selected = 0;
    while (true) {
        werase(popup);
        drawBoxSafe(popup);
        if (hasColor) {
            wattron(popup, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);
        }
        mvwprintw(popup, 1, 2, "%s", clipUiText("COMPLETED GAME HISTORY", static_cast<std::size_t>(contentW)).c_str());
        if (hasColor) {
            wattroff(popup, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);
        }

        if (!error.empty()) {
            mvwprintw(popup, 3, 2, "%s", clipUiText(error, static_cast<std::size_t>(contentW)).c_str());
        } else if (entries.empty()) {
            mvwprintw(popup, 3, 2, "%s",
                      clipUiText("No completed games recorded yet.", static_cast<std::size_t>(contentW)).c_str());
        } else {
            const int visible = std::max(1, popupH - 7);
            const int start = std::max(0, selected - visible + 1);
            for (int i = 0; i < visible && start + i < static_cast<int>(entries.size()); ++i) {
                const int index = start + i;
                const CompletedGameEntry& entry = entries[static_cast<std::size_t>(index)];
                std::ostringstream row;
                row << (index + 1) << ". " << entry.date << " - "
                    << entry.winner << " won with $" << entry.winnerScore;
                if (index == selected) {
                    wattron(popup, A_REVERSE);
                }
                mvwprintw(popup, 3 + i, 2, "%-*s", contentW,
                          clipUiText(row.str(), static_cast<std::size_t>(contentW)).c_str());
                if (index == selected) {
                    wattroff(popup, A_REVERSE);
                }
            }
            const CompletedGameEntry& entry = entries[static_cast<std::size_t>(selected)];
            std::ostringstream detail;
            detail << "Mode: " << entry.mode << " | Rounds: " << entry.rounds
                   << " | Game: " << entry.gameId;
            mvwprintw(popup, popupH - 5, 2, "%s",
                      clipUiText(detail.str(), static_cast<std::size_t>(contentW)).c_str());
            mvwprintw(popup, popupH - 4, 2, "%s",
                      clipUiText(entry.settings.empty() ? "Settings: not recorded" : "Settings: " + entry.settings,
                                 static_cast<std::size_t>(contentW)).c_str());
            mvwprintw(popup, popupH - 3, 2, "%s",
                      clipUiText(entry.playerDetails.empty() ? entry.players : entry.playerDetails,
                                 static_cast<std::size_t>(contentW)).c_str());
        }

        mvwprintw(popup, popupH - 2, 2, "%s",
                  clipUiText("Up/Down move  Enter details  ESC back",
                             static_cast<std::size_t>(contentW)).c_str());
        wrefresh(popup);
        const int ch = wgetch(popup);
        if (isCancelKey(ch)) {
            break;
        }
        if (!entries.empty() && isConfirmKey(ch)) {
            showCompletedGameDetailScreen(entries[static_cast<std::size_t>(selected)], hasColor);
            touchwin(popup);
            continue;
        }
        if (!entries.empty() && ch == KEY_UP) {
            selected = selected == 0 ? static_cast<int>(entries.size()) - 1 : selected - 1;
        } else if (!entries.empty() && ch == KEY_DOWN) {
            selected = selected + 1 >= static_cast<int>(entries.size()) ? 0 : selected + 1;
        }
    }

    delwin(popup);
    touchwin(stdscr);
    refresh();
}
