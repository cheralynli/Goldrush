#include "game.hpp"
#include "save_manager.hpp"
#include "ui.h"
#include "ui_helpers.h"
#include "ui_layout.h"
#include "input_helpers.h"
#include "tile_display.h"
#include "completed_history.h"
#include "tutorials.h"
#include "spins.hpp"

#include <algorithm>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

bool parseStrictInt(const std::string& text, int& value);

namespace {
const char* const GOLDRUSH_TITLE_ART[] = {
    "   ________        .__       .___                    .__     ",
    "  /  _____/  ____  |  |    __| _/______ __ __  _____|  |__  ",
    " /   \\  ___ /  _ \\ |  |   / __ |\\_  __ \\  |  \\/  ___/  |  \\ ",
    " \\    \\_\\  (  <_> )|  |__/ /_/ | |  | \\/  |  /\\___ \\|   Y  \\",
    "  \\______  /\\____/ |____/\\____ | |__|  |____//____  >___|  /",
    "         \\/                   \\/                  \\/     \\/ "
};
const int GOLDRUSH_TITLE_ART_LINES = static_cast<int>(sizeof(GOLDRUSH_TITLE_ART) / sizeof(GOLDRUSH_TITLE_ART[0]));
const int GOLDRUSH_TITLE_ART_WIDTH = 62;

void drawGoldrushTitleArtAt(bool hasColor, int startY) {
    int w = 0;
    int h = 0;
    getmaxyx(stdscr, h, w);
    (void)h;
    int startX = (w - GOLDRUSH_TITLE_ART_WIDTH) / 2;
    if (startY < 1) startY = 1;
    if (startX < 0) startX = 0;

    if (hasColor) {
        attron(COLOR_PAIR(GOLDRUSH_GOLD_BLACK) | A_BOLD);
    }
    for (int i = 0; i < GOLDRUSH_TITLE_ART_LINES; ++i) {
        mvprintw(startY + i, startX, "%s", GOLDRUSH_TITLE_ART[i]);
    }
    if (hasColor) {
        attroff(COLOR_PAIR(GOLDRUSH_GOLD_BLACK) | A_BOLD);
    }
}

void drawGoldrushTitleArt(bool hasColor) {
    int h = 0;
    int w = 0;
    getmaxyx(stdscr, h, w);
    (void)w;
    int startY = (h / 2) - 6;
    if (startY < 1) startY = 1;
    drawGoldrushTitleArtAt(hasColor, startY);
}

}

bool confirmTitleQuit(bool hasColor) {
    return showQuitConfirmation(hasColor);
}

//shiny number
std::vector<std::string> bigRollNumberArt(int value) {
    switch (value) {
        case 1:
            return {"   __  ", 
                    "  /_ | ", 
                    "   | | ", 
                    "   | | ",
                    "   | | ",
                    "   |_| "};
        case 2:
            return {"  ___  ", 
                    " |__ \\  ", 
                    "    ) |  ", 
                    "   / /   ",
                    "  / /___ ",
                    " /_____// "};
        case 3:
            return {"  ____  ", 
                    " |___ \\", 
                    "   __) |", 
                    "  |__ < ", 
                    "  ___) |",
                    " |____/ "};
        case 4:
            return {"  _  _   ", 
                    " | || |  ", 
                    " | || |_ ", 
                    " |__   _|", 
                    "    | |  ",
                    "    |_|   "};
        case 5:
            return {"  _____ ", 
                    " | ____|", 
                    " | |__  ", 
                    " |___ \\", 
                    "  ___) |",
                    " |____/ "};
        case 6:
            return {"   __   ", 
                    "  / /   ", 
                    " / /_   ", 
                    "| '_ \\ ", 
                    "| (_) | ",
                    "\\___/  "};
        case 7:
            return {"  ______ ", 
                    " |____  |", 
                    "     / / ", 
                    "    / /  ", 
                    "   / /   ",
                    "  /_/    "};
        case 8:
            return {"   ___   ", 
                    "  / _ \\  ", 
                    " | (_) | ", 
                    "  > _ <  ", 
                    " | (_) | ",
                    " \\___/  "};
        case 9:
            return {"   ___   ", 
                    "  / _ \\  ", 
                    " | (_) | ", 
                    " \\__, | ", 
                    "    / /  ",
                    "   /_/   "};
        case 10:
            return {"  _   ___   ", 
                    "/_ | / _ \\ ", 
                    " | || | | | ", 
                    " | || | | | ", 
                    " | || |_| | ",
                    " |_|\\___// "};
        default:
            return {std::to_string(value)};
    }
}

std::string clipMenuText(const std::string& text, std::size_t width) {
    if (text.size() <= width) {
        return text;
    }
    if (width <= 3) {
        return text.substr(0, width);
    }
    return text.substr(0, width - 3) + "...";
}

std::string formatSaveSize(std::uintmax_t sizeBytes) {
    std::ostringstream out;
    if (sizeBytes >= 1024ULL * 1024ULL) {
        out << (sizeBytes / (1024ULL * 1024ULL)) << " MB";
    } else if (sizeBytes >= 1024ULL) {
        out << (sizeBytes / 1024ULL) << " KB";
    } else {
        out << sizeBytes << " B";
    }
    return out.str();
}

std::string displayNameFromPath(const std::string& path) {
    const std::string::size_type pos = path.find_last_of("/\\");
    return pos == std::string::npos ? path : path.substr(pos + 1);
}

std::string saveMenuStatusText(const SaveFileInfo& info) {
    if (!info.metadataValid) {
        return "! Save metadata is missing or invalid. Loading may fail.";
    }
    if (info.duplicateGameId && !info.assignedTarget) {
        return "* Duplicate copy. Future saves will overwrite " + info.assignedFilename + ".";
    }
    if (info.duplicateGameId) {
        return "Canonical save for " + info.gameId + ". Duplicate copies exist.";
    }
    if (!info.gameId.empty()) {
        return "Game " + info.gameId + " | Created " + info.createdText + ".";
    }
    return "Legacy save without a persistent game ID.";
}

std::string playerRouteText(const Player& player) {
    std::string route;
    if (player.startChoice == 0) {
        route = "College";
    } else if (player.startChoice == 1) {
        route = "Career";
    } else {
        route = "Undecided";
    }

    if (player.familyChoice == 0) {
        route += " / Family";
    } else if (player.familyChoice == 1) {
        route += " / Life";
    }

    if (player.riskChoice == 0) {
        route += " / Safe";
    } else if (player.riskChoice == 1) {
        route += " / Risky";
    }

    return route;
}

void Game::applyWindowBg(WINDOW* w) const {
    apply_ui_background(w);
}

bool Game::ensureMinSize() const {
    int h, w;
    const int minHeight = minimumGameHeight();
    const int minWidth = minimumGameWidth();
    timeout(200);
    while (true) {
        getmaxyx(stdscr, h, w);
        if (h >= minHeight && w >= minWidth) {
            timeout(-1);
            return true;
        }

        if (hasColor) {
            bkgd(COLOR_PAIR(GOLDRUSH_GOLD_BLACK));
        }
        clear();
        const char* line1 = "Please enter full-screen mode"; 
        std::ostringstream line2;
        line2 << "Minimum size: " << minWidth << "x" << minHeight;
        std::ostringstream line3;
        line3 << "Current size: " << w << "x" << h;
        const char* line4 = "Press ESC to quit";
        int x1 = (w - static_cast<int>(std::strlen(line1))) / 2;
        int x2 = (w - static_cast<int>(line2.str().size())) / 2;
        int x3 = (w - static_cast<int>(line3.str().size())) / 2;
        int x4 = (w - static_cast<int>(std::strlen(line4))) / 2;
        int y = h / 2;
        if (x1 < 0) x1 = 0;
        if (x2 < 0) x2 = 0;
        if (x3 < 0) x3 = 0;
        if (x4 < 0) x4 = 0;
        mvprintw(y - 1, x1, "%s", line1);
        mvprintw(y, x2, "%s", line2.str().c_str());
        mvprintw(y + 1, x3, "%s", line3.str().c_str());
        mvprintw(y + 2, x4, "%s", line4);
        refresh();

        int ch = getch();
        if (ch == KEY_RESIZE) {
            // Terminal is being resized, continue loop to check again
            continue;
        }
        if (isCancelKey(ch)) {
            timeout(-1);
            return false;
        }
    }
}

//TODO number 4 - redraw when size becomes valid again
bool Game::recoverTerminalLayout(int currentPlayer, const std::string& msg, const std::string& detail) {
    // Force a complete UI rebuild
    destroyWindows();
    
    // Check if terminal is now valid
    if (!ensureMinSize()) {
        return false;
    }
    
    // Recreate all windows
    createWindows();
    
    // Redraw the current game state
    renderGame(currentPlayer, msg, detail);
    
    return true;
}

void Game::destroyWindows() {
    if (titleWin) { delwin(titleWin); titleWin = nullptr; }
    if (boardWin) { delwin(boardWin); boardWin = nullptr; }
    if (infoWin) { delwin(infoWin); infoWin = nullptr; }
    if (msgWin) { delwin(msgWin); msgWin = nullptr; }
    windowsValid = false;
}

void Game::createWindows() {
    // Don't early return - always recreate windows when called
    // But mark them as invalid first to ensure clean creation
    
    // Clean up any existing windows first
    if (titleWin || boardWin || infoWin || msgWin) {
        destroyWindows();  // This sets windowsValid = false
    }

    int termH, termW;
    getmaxyx(stdscr, termH, termW);
    const UILayout layout = calculateUILayout(termH, termW);
    if (layout.originY + layout.totalHeight > termH ||
        layout.originX + layout.totalWidth > termW) {
        showTerminalSizeWarning(layout.totalHeight, layout.totalWidth, hasColor);
        return;
    }

    clear();
    refresh();

    titleWin = newwin(layout.headerHeight,
                      layout.totalWidth,
                      layout.originY,
                      layout.originX);
    boardWin = newwin(layout.boardHeight,
                      layout.boardWidth,
                      layout.originY + layout.headerHeight,
                      layout.originX);
    infoWin = newwin(layout.sidePanelHeight,
                     layout.sidePanelWidth,
                     layout.originY + layout.headerHeight,
                     layout.originX + layout.boardWidth);
    msgWin = newwin(layout.messageHeight,
                    layout.totalWidth,
                    layout.originY + layout.headerHeight + layout.boardHeight,
                    layout.originX);
    if (!titleWin || !boardWin || !infoWin || !msgWin) {
        destroyWindows();
        showTerminalSizeWarning(layout.totalHeight, layout.totalWidth, hasColor);
        return;
    }

    keypad(infoWin, TRUE);
    keypad(msgWin, TRUE);
    applyWindowBg(titleWin);
    applyWindowBg(boardWin);
    applyWindowBg(infoWin);
    applyWindowBg(msgWin);
}

void Game::waitForEnter(WINDOW* w, int y, int x, const std::string& text) const {
    mvwprintw(w, y, x, "%s", text.c_str());
    wrefresh(w);
    waitForConfirmOrCancel(w);
}

void Game::drawSetupTitle() const {
    if (!msgWin) {
        return;
    }
    drawGoldrushTitleArt(hasColor);
    refresh();
}

void Game::flashSpinResult(const std::string& title, int value) const {
    static const int flashPairs[] = {
        GOLDRUSH_PLAYER_ONE,
        GOLDRUSH_PLAYER_TWO,
        GOLDRUSH_PLAYER_THREE,
        GOLDRUSH_PLAYER_FOUR
    };

    nodelay(msgWin, TRUE);
    for (int flash = 0; flash < 8; ++flash) {
        werase(msgWin);
        drawBoxSafe(msgWin);
        const int msgW = getmaxx(msgWin);
        const int contentW = std::max(1, msgW - 4);
        mvwprintw(msgWin, 1, 2, "%s", clipUiText(title, static_cast<std::size_t>(contentW)).c_str());

        const int colorPair = flashPairs[flash % 4];
        if (hasColor) {
            wattron(msgWin, COLOR_PAIR(colorPair) | A_BOLD | ((flash % 2 == 0) ? A_BLINK : 0));
        } else {
            wattron(msgWin, A_REVERSE | A_BOLD);
        }
        mvwprintw(msgWin, 2, 2, "Spin result: %d!", value);
        if (hasColor) {
            wattroff(msgWin, COLOR_PAIR(colorPair) | A_BOLD | ((flash % 2 == 0) ? A_BLINK : 0));
        } else {
            wattroff(msgWin, A_REVERSE | A_BOLD);
        }
        wrefresh(msgWin);

        const int ch = wgetch(msgWin);
        if (ch != ERR) {
            break;
        }
        napms(90);
    }
    nodelay(msgWin, FALSE);
}

//show shiny number
void Game::showRollResultPopup(int value) const {
    int h = 0;
    int w = 0;
    getmaxyx(stdscr, h, w);
    (void)h;

    if (titleWin) {
        touchwin(titleWin);
        wrefresh(titleWin);
    }
    if (boardWin) {
        touchwin(boardWin);
        wrefresh(boardWin);
    }
    if (infoWin) {
        touchwin(infoWin);
        wrefresh(infoWin);
    }
    if (msgWin) {
        touchwin(msgWin);
        wrefresh(msgWin);
    }
    //here
    const std::vector<std::string> banner = bigRollNumberArt(value);
    const std::string title = "YOU ROLLED A";
    static const int flashPairs[] = {
        GOLDRUSH_PLAYER_ONE,
        GOLDRUSH_PLAYER_TWO,
        GOLDRUSH_PLAYER_THREE,
        GOLDRUSH_PLAYER_FOUR
    };
    int contentWidth = static_cast<int>(title.size());
    for (std::size_t i = 0; i < banner.size(); ++i) {
        contentWidth = std::max(contentWidth, static_cast<int>(banner[i].size()));
    }

    const int popupW = std::min(std::max(32, contentWidth + 8), w - 2);
    const int popupH = static_cast<int>(banner.size()) + 6;
    WINDOW* popup = createCenteredWindow(popupH, popupW, 10, 28);
    if (!popup) {
        showTerminalSizeWarning(popupH, 28, hasColor);
        return;
    }
    const int innerWidth = getmaxx(popup) - 4;
    for (int flash = 0; flash < 10; ++flash) {
        werase(popup);
        drawBoxSafe(popup);
        const int colorPair = flashPairs[flash % 4];
        if (hasColor) {
            wattron(popup, COLOR_PAIR(colorPair) | A_BOLD | ((flash % 2 == 0) ? A_BLINK : 0));
        } else {
            wattron(popup, A_BOLD | ((flash % 2 == 0) ? A_REVERSE : 0));
        }
        const std::string clippedTitle = clipUiText(title, static_cast<std::size_t>(std::max(1, innerWidth)));
        mvwprintw(popup, 1, 2 + std::max(0, (innerWidth - static_cast<int>(clippedTitle.size())) / 2),
                  "%s", clippedTitle.c_str());
        for (std::size_t i = 0; i < banner.size(); ++i) {
            const std::string line = clipUiText(banner[i], static_cast<std::size_t>(std::max(1, innerWidth)));
            mvwprintw(popup,
                      3 + static_cast<int>(i),
                      2 + std::max(0, (innerWidth - static_cast<int>(line.size())) / 2),
                      "%s",
                      line.c_str());
        }
        if (hasColor) {
            wattroff(popup, COLOR_PAIR(colorPair) | A_BOLD | ((flash % 2 == 0) ? A_BLINK : 0));
        } else {
            wattroff(popup, A_BOLD | ((flash % 2 == 0) ? A_REVERSE : 0));
        }
        wrefresh(popup);
        napms(120);
    }
    delwin(popup);

    if (titleWin) {
        touchwin(titleWin);
        wrefresh(titleWin);
    }
    if (boardWin) {
        touchwin(boardWin);
        wrefresh(boardWin);
    }
    if (infoWin) {
        touchwin(infoWin);
        wrefresh(infoWin);
    }
    if (msgWin) {
        touchwin(msgWin);
        wrefresh(msgWin);
    }
}

bool Game::promptForFilename(const std::string& action,
                             const std::string& defaultName,
                             std::string& filename) {
    std::ostringstream prompt;
    prompt << action << " in saves/ [" << defaultName << "]: ";
    const std::string promptText = prompt.str();

    noecho();
    curs_set(1);
    werase(msgWin);
    drawBoxSafe(msgWin);
    const int msgW = getmaxx(msgWin);
    const std::string clippedPrompt = clipMenuText(promptText, static_cast<std::size_t>(std::max(8, msgW - 4)));
    mvwprintw(msgWin, 1, 2, "%s", clippedPrompt.c_str());
    mvwprintw(msgWin,
              2,
              2,
              "%s",
              clipMenuText("Press ENTER for the default name or type a new filename.",
                           static_cast<std::size_t>(std::max(8, msgW - 4))).c_str());
    wmove(msgWin, 1, std::min(msgW - 2, 2 + static_cast<int>(clippedPrompt.size())));
    wrefresh(msgWin);

    char buffer[260] = {0};
    const bool confirmed = readLineOrCancel(msgWin, buffer, 259);

    noecho();
    curs_set(0);
    if (!confirmed) {
        return false;
    }

    filename = buffer;
    if (filename.empty()) {
        filename = defaultName;
    }
    return true;
}

bool Game::chooseSaveFileToLoad(SaveFileInfo& selected) {
    SaveManager saveManager;
    std::string error;
    const std::vector<SaveFileInfo> saves = saveManager.listSaveFiles(error);
    if (!error.empty()) {
        showInfoPopup("Load failed", error);
        return false;
    }
    if (saves.empty()) {
        showInfoPopup("Load Game", "No save files were found in saves/.");
        return false;
    }

    int termH, termW;
    getmaxyx(stdscr, termH, termW);
    const int popupH = std::min(20, termH - 4);
    const int popupW = std::min(108, std::max(64, termW - 4));
    WINDOW* popup = createCenteredWindow(popupH, popupW, 10, 64);
    if (!popup) {
        showTerminalSizeWarning(10, 64, hasColor);
        return false;
    }
    keypad(popup, TRUE);

    int popupActualH = 0;
    int popupActualW = 0;
    getmaxyx(popup, popupActualH, popupActualW);

    const int listTop = 4;
    const int listBottom = popupActualH - 4;
    const int visibleRows = std::max(1, listBottom - listTop + 1);
    const int availableWidth = popupActualW - 4;
    const int modifiedWidth = 19;
    const int sizeWidth = 9;
    int fileWidth = availableWidth - modifiedWidth - sizeWidth - 4;
    if (fileWidth < 20) {
        fileWidth = 20;
    }

    int selectedIndex = 0;
    int topIndex = 0;

    while (true) {
        if (selectedIndex < topIndex) {
            topIndex = selectedIndex;
        }
        if (selectedIndex >= topIndex + visibleRows) {
            topIndex = selectedIndex - visibleRows + 1;
        }

        werase(popup);
        drawBoxSafe(popup);
        mvwprintw(popup, 1, 2, "Load Game");
        mvwprintw(popup, 2, 2, "%s",
                  clipMenuText("Arrow keys move  Enter load  PgUp/PgDn scroll  ESC cancel",
                               static_cast<std::size_t>(availableWidth)).c_str());
        mvwprintw(popup, 3, 2, "%-*s  %-19s  %9s", fileWidth, "File", "Modified", "Size");

        for (int row = 0; row < visibleRows; ++row) {
            const int index = topIndex + row;
            if (index >= static_cast<int>(saves.size())) {
                break;
            }

            const SaveFileInfo& info = saves[static_cast<std::size_t>(index)];
            std::string displayFilename = info.filename;
            if (!info.metadataValid) {
                displayFilename = "! " + displayFilename;
            } else if (info.duplicateGameId && !info.assignedTarget) {
                displayFilename = "* " + displayFilename;
            }
            const std::string filename = clipMenuText(displayFilename, static_cast<std::size_t>(fileWidth));
            const std::string sizeText = clipMenuText(formatSaveSize(info.sizeBytes), static_cast<std::size_t>(sizeWidth));

            if (index == selectedIndex) {
                wattron(popup, A_REVERSE);
            }
            mvwprintw(popup, listTop + row, 2, "%-*s  %-19s  %9s",
                      fileWidth,
                      filename.c_str(),
                      info.modifiedText.c_str(),
                      sizeText.c_str());
            if (index == selectedIndex) {
                wattroff(popup, A_REVERSE);
            }
        }

        const int shownFrom = topIndex + 1;
        const int shownTo = std::min(topIndex + visibleRows, static_cast<int>(saves.size()));
        mvwprintw(popup, popupActualH - 3, 2, "%s",
                  clipMenuText(saveMenuStatusText(saves[static_cast<std::size_t>(selectedIndex)]),
                               static_cast<std::size_t>(availableWidth)).c_str());
        mvwprintw(popup, popupActualH - 2, 2, "Showing %d-%d of %d",
                  shownFrom, shownTo, static_cast<int>(saves.size()));
        mvwprintw(popup, popupActualH - 2, popupActualW / 2, "%s",
                  clipMenuText(saves[static_cast<std::size_t>(selectedIndex)].filename,
                               static_cast<std::size_t>(std::max(1, popupActualW / 2 - 4))).c_str());
        wrefresh(popup);

        const int ch = wgetch(popup);
        if (ch == KEY_UP) {
            if (selectedIndex > 0) {
                --selectedIndex;
            }
        } else if (ch == KEY_DOWN) {
            if (selectedIndex + 1 < static_cast<int>(saves.size())) {
                ++selectedIndex;
            }
        } else if (ch == KEY_PPAGE) {
            selectedIndex = std::max(0, selectedIndex - visibleRows);
        } else if (ch == KEY_NPAGE) {
            selectedIndex = std::min(static_cast<int>(saves.size()) - 1, selectedIndex + visibleRows);
        } else if (isConfirmKey(ch)) {
            selected = saves[static_cast<std::size_t>(selectedIndex)];
            delwin(popup);
            return true;
        } else if (isCancelKey(ch) || ch == KEY_RESIZE) {
            delwin(popup);
            return false;
        }
    }
}

Game::StartChoice Game::showStartScreen() {
    //const char* lines[] = {
    //  "  ________         .__       .___                       .__      ",
    //  " /  _____/    ____ |  |    __| _/______ __ __     _____ |  |__   ",
    //  "/   \\  ___  /  _\\|  |   / __ |\\_  __ \\  |  \\/  ___/|     \\ ",
    //  "\\   \\_\\  (  <_> )  |__/ /_/ |  |  | \\/  |  /\\___ \\|   Y  \\",
    //  " \\______  /\\____/|____/\\____|  |__|   |____/  /____  >___|  / ",
    //  "        \\/                  \\/                     \\/    \\/  "
    //};
    //const int artLines = static_cast<int>(sizeof(lines) / sizeof(lines[0]));
    //const int artW = 60;
    // The title screen is a two-step state machine: first choose new/load/quit,
    // then choose the ruleset for a new game.
    bool choosingMode = false;
    int highlightedMode = 0;

    while (true) {
        int h, w;
        getmaxyx(stdscr, h, w);
        clear();
        if (hasColor) bkgd(COLOR_PAIR(GOLDRUSH_GOLD_BLACK));
        drawGoldrushTitleArt(hasColor);

        int startY = (h / 2) - 6;
        if (startY < 1) startY = 1;

        if (hasColor) wattron(stdscr, COLOR_PAIR(GOLDRUSH_BROWN_SAND));
        if (!choosingMode) {
            const std::string subtitle = "A Hasbro-style Life Journey";
    const std::string actions = "N New   L Load   G Guide   H History   ESC Quit";
            mvprintw(startY + 11, std::max(0, (w - static_cast<int>(subtitle.size())) / 2), "%s", subtitle.c_str());
            mvprintw(startY + 13, std::max(0, (w - static_cast<int>(actions.size())) / 2), "%s", actions.c_str());
        } else {
            const std::string help = "ENTER select    ESC back";
            mvprintw(startY + 11, std::max(0, (w - static_cast<int>(help.size())) / 2), "%s", help.c_str());
            const char* normal = "Normal Mode";
            const char* custom = "Custom Mode";
            const char* normalDesc = "Every optional system is enabled for the full game.";
            const char* customDesc = "Open the rules page and toggle features before starting.";
            int rowY = startY + 13;
            int normalX = (w / 2) - 18;
            int customX = (w / 2) + 4;
            if (highlightedMode == 0) attron(A_REVERSE);
            mvprintw(rowY, normalX, "%s", normal);
            if (highlightedMode == 0) attroff(A_REVERSE);
            if (highlightedMode == 1) attron(A_REVERSE);
            mvprintw(rowY, customX, "%s", custom);
            if (highlightedMode == 1) attroff(A_REVERSE);
            mvprintw(startY + 15, (w - 56) / 2, "%-56s", highlightedMode == 0 ? normalDesc : customDesc);
        }
        if (hasColor) wattroff(stdscr, COLOR_PAIR(GOLDRUSH_BROWN_SAND));
        refresh();

        int ch = getch();
        if (!choosingMode) {
            if (ch == 'n' || ch == 'N' || ch == 's' || ch == 'S') {
                settings = createLifeModeSettings();
                rules = makeNormalRules();
                if (!chooseBoardViewMode()) {
                    continue;
                }
                return START_NEW_GAME;
            }
            if (ch == 'l' || ch == 'L') return START_LOAD_GAME;
            if (ch == 'g' || ch == 'G') {
                showGuidePopup();
                continue;
            }
            if (ch == 'h' || ch == 'H') {
                showCompletedGameHistoryScreen(hasColor);
                continue;
            }
            if (isCancelKey(ch)) {
                if (confirmTitleQuit(hasColor)) {
                    return START_QUIT_GAME;
                }
                continue;
            }
        } else {
            if (ch == KEY_LEFT || ch == KEY_UP) {
                highlightedMode = highlightedMode == 0 ? 1 : 0;
                continue;
            }
            if (ch == KEY_RIGHT || ch == KEY_DOWN) {
                highlightedMode = highlightedMode == 1 ? 0 : 1;
                continue;
            }
            if (isCancelKey(ch)) {
                choosingMode = false;
                continue;
            }
            if (isConfirmKey(ch)) {
                if (highlightedMode == 0) {
                    rules = makeNormalRules();
                    return START_NEW_GAME;
                }

                rules = makeCustomRules();
                if (configureCustomRules()) {
                    return START_NEW_GAME;
                }
                choosingMode = false;
                continue;
            }
        }
        if (ch == KEY_RESIZE && !ensureMinSize()) return START_QUIT_GAME;
    }
}

bool Game::chooseBoardViewMode() {
    const char* primaryNames[] = {
        "1860s Board",
        "Other Board Styles"
    };
    const char* primaryDescriptions[] = {
        "Use the 1860-inspired checkered board and movement view.",
        "Pick Follow Camera or Classic / Full Board in the next step."
    };
    const char* otherNames[] = {
        "Follow Camera Mode",
        "Classic / Full Board Mode"
    };
    const char* otherDescriptions[] = {
        "Zoomed route view centered on the current player.",
        "Shows the whole route with the classic board layout."
    };

    auto chooseOtherStylesFallback = [&]() -> int {
        int otherSelected = boardViewMode == BoardViewMode::ClassicFull ? 1 : 0;
        while (true) {
            const int w = getmaxx(stdscr);
            const int popupW = std::min(78, std::max(60, w - 6));
            const int popupH = 11;
            WINDOW* popup = createCenteredWindow(popupH, popupW, 11, 48);
            if (!popup) {
                showTerminalSizeWarning(11, 48, hasColor);
                return -1;
            }
            keypad(popup, TRUE);
            int popupActualH = 0;
            int popupActualW = 0;
            getmaxyx(popup, popupActualH, popupActualW);

            while (true) {
                werase(popup);
                drawBoxSafe(popup);
                if (hasColor) wattron(popup, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);
                mvwprintw(popup, 1, 2, "Other Board Styles");
                if (hasColor) wattroff(popup, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);

                for (int i = 0; i < 2; ++i) {
                    const int row = 3 + (i * 2);
                    if (otherSelected == i) wattron(popup, A_REVERSE | A_BOLD);
                    mvwprintw(popup, row, 4, "%s",
                              clipUiText(std::to_string(i + 1) + ". " + otherNames[i],
                                         static_cast<std::size_t>(std::max(1, popupActualW - 6))).c_str());
                    if (otherSelected == i) wattroff(popup, A_REVERSE | A_BOLD);
                    mvwprintw(popup,
                              row + 1,
                              7,
                              "%s",
                              clipUiText(otherDescriptions[i], static_cast<std::size_t>(popupActualW - 10)).c_str());
                }

                mvwprintw(popup, popupActualH - 3, 2, "%s",
                          clipUiText("Use UP/DOWN or A/D. ENTER select. ESC back.",
                                     static_cast<std::size_t>(std::max(1, popupActualW - 4))).c_str());
                mvwprintw(popup, popupActualH - 2, 2, "%s",
                          clipUiText(std::string("Selected: ") + otherNames[otherSelected],
                                     static_cast<std::size_t>(std::max(1, popupActualW - 4))).c_str());
                wrefresh(popup);

                const int ch = wgetch(popup);
                if (ch == KEY_RESIZE) {
                    delwin(popup);
                    if (!ensureMinSize()) {
                        return -1;
                    }
                    break;
                }
                if (ch == KEY_UP || ch == KEY_DOWN ||
                    ch == KEY_LEFT || ch == KEY_RIGHT ||
                    ch == 'a' || ch == 'A' ||
                    ch == 'd' || ch == 'D' ||
                    ch == 'w' || ch == 'W' ||
                    ch == 's' || ch == 'S') {
                    otherSelected = (otherSelected + 1) % 2;
                } else if (ch == '1' || ch == '2') {
                    otherSelected = ch - '1';
                } else if (isCancelKey(ch)) {
                    delwin(popup);
                    return -1;
                } else if (isConfirmKey(ch)) {
                    delwin(popup);
                    return otherSelected;
                }
            }
        }
    };

    int selected = 0;

    if (!msgWin || !titleWin) {
        while (true) {
            int h = 0;
            int w = 0;
            getmaxyx(stdscr, h, w);
            const UILayout layout = calculateUILayout(h, w);
            clear();
            if (hasColor) {
                bkgd(COLOR_PAIR(GOLDRUSH_GOLD_BLACK));
            }
            drawGoldrushTitleArt(hasColor);

            const int boxH = layout.messageHeight;
            const int boxW = layout.totalWidth;
            const int boxY = layout.originY + layout.headerHeight + layout.boardHeight;
            const int boxX = layout.originX;
            WINDOW* box = newwin(boxH, boxW, boxY, boxX);
            if (!box) {
                return false;
            }
            applyWindowBg(box);
            werase(box);
            drawBoxSafe(box);
            const int contentW = std::max(8, boxW - 4);
            mvwprintw(box, 1, 2, "%s",
                      clipUiText("Choose Board Display", static_cast<std::size_t>(contentW)).c_str());
            for (int i = 0; i < 2; ++i) {
                if (selected == i) {
                    wattron(box, A_REVERSE | A_BOLD);
                }
                const std::string lineText = std::string("- ") + primaryNames[i] +
                                             (i == selected ? std::string("  ") + primaryDescriptions[i] : std::string());
                mvwprintw(box, 2 + i, 2, "%-*s", contentW,
                          clipUiText(lineText, static_cast<std::size_t>(contentW)).c_str());
                if (selected == i) {
                    wattroff(box, A_REVERSE | A_BOLD);
                }
            }
            refresh();
            wrefresh(box);

            const int ch = getch();
            delwin(box);
            if (ch == KEY_RESIZE) {
                if (!ensureMinSize()) {
                    return false;
                }
                continue;
            }
            if (ch == KEY_LEFT || ch == KEY_RIGHT ||
                ch == KEY_UP || ch == KEY_DOWN ||
                ch == 'a' || ch == 'A' || ch == 'd' || ch == 'D' ||
                ch == 'w' || ch == 'W' || ch == 's' || ch == 'S') {
                selected = (selected + 1) % 2;
            } else if (ch == '1' || ch == '2') {
                selected = ch - '1';
            } else if (ch == 27) {
                return false;
            } else if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
                if (selected == 0) {
                    boardViewMode = BoardViewMode::Mode1860;
                    return true;
                }
                const int otherChoice = chooseOtherStylesFallback();
                if (otherChoice < 0) {
                    continue;
                }
                boardViewMode = otherChoice == 0
                    ? BoardViewMode::FollowCamera
                    : BoardViewMode::ClassicFull;
                return true;
            }
        }
    }

    if (msgWin && titleWin) {
        if (boardWin) {
            werase(boardWin);
            drawBoxSafe(boardWin);
            wrefresh(boardWin);
        }
        if (infoWin) {
            werase(infoWin);
            drawBoxSafe(infoWin);
            wrefresh(infoWin);
        }
        renderHeader();

        while (true) {
            int msgH = 0;
            int msgW = 0;
            getmaxyx(msgWin, msgH, msgW);
            (void)msgH;
            const int contentW = std::max(8, msgW - 4);

            werase(msgWin);
            drawBoxSafe(msgWin);
            mvwprintw(msgWin, 1, 2, "%s",
                      clipUiText("Choose Board Display", static_cast<std::size_t>(contentW)).c_str());

            for (int i = 0; i < 2; ++i) {
                if (selected == i) {
                    wattron(msgWin, A_REVERSE | A_BOLD);
                }
                const std::string lineText = std::string("- ") + primaryNames[i] +
                                             (i == selected ? std::string("  ") + primaryDescriptions[i] : std::string());
                mvwprintw(msgWin, 2 + i, 2, "%-*s",
                          contentW,
                          clipUiText(lineText, static_cast<std::size_t>(contentW)).c_str());
                if (selected == i) {
                    wattroff(msgWin, A_REVERSE | A_BOLD);
                }
            }
            wrefresh(msgWin);

            const int ch = wgetch(msgWin);
            if (ch == KEY_RESIZE) {
                if (!ensureMinSize()) {
                    return false;
                }
                destroyWindows();
                createWindows();
                renderHeader();
                continue;
            }
            if (ch == KEY_LEFT || ch == KEY_RIGHT ||
                ch == KEY_UP || ch == KEY_DOWN ||
                ch == 'a' || ch == 'A' ||
                ch == 'd' || ch == 'D' ||
                ch == 'w' || ch == 'W' ||
                ch == 's' || ch == 'S') {
                selected = (selected + 1) % 2;
            } else if (ch == '1' || ch == '2') {
                selected = ch - '1';
            } else if (ch == 27) {
                return false;
            } else if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
                if (selected == 0) {
                    boardViewMode = BoardViewMode::Mode1860;
                    return true;
                }
                const int otherChoice = chooseOtherStylesFallback();
                if (otherChoice < 0) {
                    renderHeader();
                    continue;
                }
                boardViewMode = otherChoice == 0
                    ? BoardViewMode::FollowCamera
                    : BoardViewMode::ClassicFull;
                return true;
            }
        }
    }

    while (true) {
        int h = 0;
        int w = 0;
        getmaxyx(stdscr, h, w);
        (void)h;
        const int popupW = std::min(78, std::max(60, w - 6));
        const int popupH = 11;
        WINDOW* popup = createCenteredWindow(popupH, popupW, 11, 48);
        if (!popup) {
            showTerminalSizeWarning(11, 48, hasColor);
            return false;
        }
        keypad(popup, TRUE);
        int popupActualH = 0;
        int popupActualW = 0;
        getmaxyx(popup, popupActualH, popupActualW);

        while (true) {
            werase(popup);
            drawBoxSafe(popup);
            if (hasColor) wattron(popup, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);
            mvwprintw(popup, 1, 2, "Choose Board Display");
            if (hasColor) wattroff(popup, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);

            for (int i = 0; i < 2; ++i) {
                const int row = 3 + (i * 2);
                if (selected == i) wattron(popup, A_REVERSE | A_BOLD);
                mvwprintw(popup, row, 4, "%s",
                          clipUiText(std::to_string(i + 1) + ". " + primaryNames[i],
                                     static_cast<std::size_t>(std::max(1, popupActualW - 6))).c_str());
                if (selected == i) wattroff(popup, A_REVERSE | A_BOLD);
                mvwprintw(popup,
                          row + 1,
                          7,
                          "%s",
                          clipUiText(primaryDescriptions[i], static_cast<std::size_t>(popupActualW - 10)).c_str());
            }

            mvwprintw(popup, popupActualH - 3, 2, "%s",
                      clipUiText("Use UP/DOWN or A/D. ENTER select. ESC back.",
                                 static_cast<std::size_t>(std::max(1, popupActualW - 4))).c_str());
            mvwprintw(popup, popupActualH - 2, 2, "%s",
                      clipUiText(std::string("Selected: ") + primaryNames[selected],
                                 static_cast<std::size_t>(std::max(1, popupActualW - 4))).c_str());
            wrefresh(popup);

            const int ch = wgetch(popup);
            if (ch == KEY_RESIZE) {
                delwin(popup);
                if (!ensureMinSize()) {
                    return false;
                }
                break;
            }
            if (ch == KEY_UP || ch == KEY_DOWN ||
                ch == KEY_LEFT || ch == KEY_RIGHT ||
                ch == 'a' || ch == 'A' ||
                ch == 'd' || ch == 'D' ||
                ch == 'w' || ch == 'W' ||
                ch == 's' || ch == 'S') {
                selected = (selected + 1) % 2;
            } else if (ch == '1' || ch == '2') {
                selected = ch - '1';
            } else if (isCancelKey(ch)) {
                delwin(popup);
                return false;
            } else if (isConfirmKey(ch)) {
                delwin(popup);
                if (selected == 0) {
                    boardViewMode = BoardViewMode::Mode1860;
                    return true;
                }
                const int otherChoice = chooseOtherStylesFallback();
                if (otherChoice < 0) {
                    break;
                }
                boardViewMode = otherChoice == 0
                    ? BoardViewMode::FollowCamera
                    : BoardViewMode::ClassicFull;
                return true;
            }
        }
    }
}

bool Game::configureCustomRules() {
    WINDOW* popup = createCenteredWindow(18, 72, 18, 56);
    if (!popup) {
        showTerminalSizeWarning(18, 56, hasColor);
        return false;
    }
    keypad(popup, TRUE);
    int popupH = 0;
    int popupW = 0;
    getmaxyx(popup, popupH, popupW);
    const int contentW = std::max(1, popupW - 4);

    struct ToggleRow {
        const char* label;
        bool* value;
    };

    std::vector<ToggleRow> rows;
    rows.push_back({"Tutorial", &rules.toggles.tutorialEnabled});
    rows.push_back({"Family path", &rules.toggles.familyPathEnabled});
    rows.push_back({"Night school", &rules.toggles.nightSchoolEnabled});
    rows.push_back({"Risky route", &rules.toggles.riskyRouteEnabled});
    rows.push_back({"Investments", &rules.toggles.investmentEnabled});
    rows.push_back({"Pets", &rules.toggles.petsEnabled});
    rows.push_back({"Spin to Win", &rules.toggles.spinToWinEnabled});
    rows.push_back({"Electronic banking theme", &rules.toggles.electronicBankingEnabled});
    rows.push_back({"House sale spins", &rules.toggles.houseSaleSpinEnabled});
    rows.push_back({"Retirement bonuses", &rules.toggles.retirementBonusesEnabled});

    int highlight = 0;
    const int startRowIndex = static_cast<int>(rows.size());

    while (true) {
        werase(popup);
        drawBoxSafe(popup);
        mvwprintw(popup, 1, 2, "Custom Mode");
        mvwprintw(popup, 2, 2, "%s",
                  clipUiText("Toggle rules with SPACE or ENTER. Select Start Game when ready.",
                             static_cast<std::size_t>(contentW)).c_str());

        for (size_t i = 0; i < rows.size(); ++i) {
            if (static_cast<int>(i) == highlight) wattron(popup, A_REVERSE);
            mvwprintw(popup, 4 + static_cast<int>(i), 2, "%s",
                      clipUiText(std::string("[") + (*rows[i].value ? 'x' : ' ') + "] " + rows[i].label,
                                 static_cast<std::size_t>(contentW)).c_str());
            if (static_cast<int>(i) == highlight) wattroff(popup, A_REVERSE);
        }

        if (highlight == startRowIndex) wattron(popup, A_REVERSE);
        mvwprintw(popup, popupH - 3, 2, "Start Game");
        if (highlight == startRowIndex) wattroff(popup, A_REVERSE);
        mvwprintw(popup, popupH - 2, 2, "%s",
                  clipUiText("Up/Down move  Enter/Space toggle  ESC close config",
                             static_cast<std::size_t>(contentW)).c_str());
        wrefresh(popup);

        int ch = wgetch(popup);
        if (ch == KEY_UP) {
            highlight = highlight == 0 ? startRowIndex : highlight - 1;
        } else if (ch == KEY_DOWN) {
            highlight = highlight == startRowIndex ? 0 : highlight + 1;
        } else if (isCancelKey(ch)) {
            delwin(popup);
            return false;
        } else if (ch == KEY_RESIZE) {
            delwin(popup);
            return false;
        } else if (isConfirmKey(ch, true)) {
            if (highlight == startRowIndex) {
                break;
            }
            *rows[highlight].value = !*rows[highlight].value;
        }
    }

    delwin(popup);

    rules.components.investCards = rules.toggles.investmentEnabled ? 4 : 0;
    rules.components.petCards = rules.toggles.petsEnabled ? 12 : 0;
    rules.components.spinToWinTokens = rules.toggles.spinToWinEnabled ? 5 : 0;
    return true;
}

void Game::showControlsPopup() const {
    WINDOW* popup = createCenteredWindow(16, 70, 16, 50);
    if (!popup) {
        showTerminalSizeWarning(16, 50, hasColor);
        return;
    }
    drawBoxSafe(popup);
    int popupH = 0;
    int popupW = 0;
    getmaxyx(popup, popupH, popupW);
    const int contentW = std::max(1, popupW - 4);
    mvwprintw(popup, 1, 2, "Controls");
    const std::vector<std::string> lines{
        "ENTER  Confirm a menu or start your turn spin",
        "SPACE  Hold/release to spin the wheel",
        "UP/DN  Move through menus and custom mode toggles",
        "TAB    Open the player scoreboard and minimap",
        "G      Open the tile abbreviation guide",
        "B      Open sabotage and defense actions",
        "S      Save the current game",
        "K/?    Open this controls popup",
        "ESC    Open the quit menu / back out",
        "STOP spaces end movement immediately.",
        "College tuition is paid as soon as College route is chosen."
    };
    for (int i = 0; i < static_cast<int>(lines.size()) && 3 + i < popupH - 2; ++i) {
        mvwprintw(popup, 3 + i, 2, "%s",
                  clipUiText(lines[static_cast<std::size_t>(i)],
                             static_cast<std::size_t>(contentW)).c_str());
    }
    mvwprintw(popup, popupH - 2, 2, "%s",
              clipUiText("Press ENTER", static_cast<std::size_t>(contentW)).c_str());
    wrefresh(popup);
    waitForEnter(popup, popupH - 2, std::min(popupW - 2, 15), "");
    delwin(popup);
}

void Game::showScoreboardPopup() const {
    int h, w;
    getmaxyx(stdscr, h, w);
    const int popupH = std::min(28, std::max(18, h - 4));
    const int popupW = std::min(104, std::max(92, w - 8));
    WINDOW* popup = createCenteredWindow(popupH, popupW, 18, 70);
    if (!popup) {
        showTerminalSizeWarning(18, 70, hasColor);
        return;
    }
    keypad(popup, TRUE);
    drawBoxSafe(popup);
    int actualPopupH = 0;
    int actualPopupW = 0;
    getmaxyx(popup, actualPopupH, actualPopupW);

    WINDOW* scoreWin = derwin(popup, actualPopupH - 4, 56, 2, 2);
    WINDOW* mapWin = derwin(popup, actualPopupH - 4, actualPopupW - 61, 2, 59);
    if (!scoreWin || !mapWin) {
        if (mapWin) delwin(mapWin);
        if (scoreWin) delwin(scoreWin);
        delwin(popup);
        showTerminalSizeWarning(18, 70, hasColor);
        return;
    }
    applyWindowBg(scoreWin);
    applyWindowBg(mapWin);

    blinkIndicator(popup, 1, 2, "Scoreboard + Minimap", hasColor, GOLDRUSH_GOLD_SAND, 2, 0, actualPopupW - 4);

    werase(scoreWin);
    drawBoxSafe(scoreWin);
    const int scoreWinH = getmaxy(scoreWin);
    const int scoreWinW = getmaxx(scoreWin);
    mvwprintw(scoreWin, 1, 2, "SCOREBOARD");
    mvwprintw(scoreWin, 2, 2, "Rk %-10s %-7s %-7s %-7s %-4s %-7s",
              "Player", "Type", "Space", "Cash", "Loan", "Worth");

    std::vector<int> order;
    for (size_t i = 0; i < players.size(); ++i) {
        order.push_back(static_cast<int>(i));
    }
    std::sort(order.begin(), order.end(), [this](int a, int b) {
        return calculateFinalWorth(players[a]) > calculateFinalWorth(players[b]);
    });

    for (size_t row = 0; row < order.size(); ++row) {
        const int playerIndex = order[row];
        const Player& player = players[static_cast<std::size_t>(playerIndex)];
        const Tile& tile = board.tileAt(player.tile);
        const int y = 4 + static_cast<int>(row);
        const std::string name = clipMenuText(player.name, 10);
        const std::string tileText = clipMenuText(std::to_string(player.tile) + " " + getTileAbbreviation(tile), 7);
        const std::string typeText = player.type == PlayerType::CPU
            ? clipMenuText("CPU-" + cpuDifficultyLabel(player.cpuDifficulty), 7)
            : "Human";

        if (y >= scoreWinH - 5) {
            break;
        }

        if (playerIndex == currentPlayerIndex) {
            wattron(scoreWin, A_REVERSE);
        }
        mvwprintw(scoreWin,
                  y,
                  2,
                  "%-2d %-10s %-7s %-7s $%-6d %-4d $%-6d",
                  static_cast<int>(row + 1),
                  name.c_str(),
                  typeText.c_str(),
                  tileText.c_str(),
                  player.cash,
                  player.loans,
                  calculateFinalWorth(player));
        if (playerIndex == currentPlayerIndex) {
            wattroff(scoreWin, A_REVERSE);
        }
    }

    mvwprintw(scoreWin, scoreWinH - 4, 2, "%-*s", scoreWinW - 4, "Highlighted row has the current turn.");
    mvwprintw(scoreWin, scoreWinH - 3, 2, "Route: %s",
              clipMenuText(playerRouteText(players[currentPlayerIndex]), scoreWinW - 11).c_str());
    mvwprintw(scoreWin, scoreWinH - 2, 2, "Now: %s on %d - %s",
              players[currentPlayerIndex].name.c_str(),
              players[currentPlayerIndex].tile,
              clipMenuText(getTileDisplayName(board.tileAt(players[currentPlayerIndex].tile)), scoreWinW - 18).c_str());
    wrefresh(scoreWin);

    drawMinimapPanel(mapWin, board, players, currentPlayerIndex);

    mvwprintw(popup, actualPopupH - 2, 2, "Close with TAB, ENTER, or ESC.");
    wrefresh(popup);

    while (true) {
        const int ch = wgetch(popup);
        if (ch == '\t' || isConfirmKey(ch) || isCancelKey(ch)) {
            break;
        }
    }

    delwin(mapWin);
    delwin(scoreWin);
    delwin(popup);
}

void Game::showTileGuidePopup() const {
    int h, w;
    getmaxyx(stdscr, h, w);
    (void)h;
    (void)w;
    const std::vector<std::string> legend = board.tutorialLegend();
    const int popupH = 24;
    const int popupW = 82;
    WINDOW* popup = createCenteredWindow(popupH, popupW, 16, 54);
    if (!popup) {
        showTerminalSizeWarning(16, 54, hasColor);
        return;
    }
    drawBoxSafe(popup);
    int actualH = 0;
    int actualW = 0;
    getmaxyx(popup, actualH, actualW);

    blinkIndicator(popup, 1, 2, "Quick Tutorial: Tile Guide", hasColor, GOLDRUSH_GOLD_SAND, 2, 0, actualW - 4);
    mvwprintw(popup, 2, 2, "%s",
              clipUiText("Board abbreviations show as Full Name (ABBR)",
                         static_cast<std::size_t>(std::max(1, actualW - 4))).c_str());

    const bool twoColumns = actualW >= 78;
    const int rowsPerColumn = twoColumns ? 10 : std::max(1, actualH - 6);
    const int columnWidth = twoColumns ? std::max(1, (actualW - 6) / 2) : std::max(1, actualW - 4);
    for (size_t i = 0; i < legend.size(); ++i) {
        const int column = twoColumns ? static_cast<int>(i) / rowsPerColumn : 0;
        if (column > 1) {
            break;
        }
        const int columnX = twoColumns ? (2 + column * (columnWidth + 2)) : 2;
        const int rowY = 4 + static_cast<int>(i % rowsPerColumn);
        if (rowY >= actualH - 2) {
            break;
        }
        mvwprintw(popup, rowY, columnX, "%s",
                  clipUiText(legend[i], static_cast<std::size_t>(columnWidth)).c_str());
    }

    mvwprintw(popup, actualH - 2, 2, "%s",
              clipUiText("Press ENTER", static_cast<std::size_t>(std::max(1, actualW - 4))).c_str());
    wrefresh(popup);
    waitForEnter(popup, actualH - 2, 15, "");
    delwin(popup);
}

int Game::findPlayerIndex(const Player& player) const {
    for (size_t i = 0; i < players.size(); ++i) {
        if (&players[i] == &player) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int Game::chooseSabotageTarget(int attackerIndex) {
    std::vector<int> candidates;
    for (size_t i = 0; i < players.size(); ++i) {
        if (static_cast<int>(i) != attackerIndex && !players[i].retired) {
            candidates.push_back(static_cast<int>(i));
        }
    }
    if (candidates.empty()) {
        showInfoPopup("Sabotage", "No valid targets.");
        return -1;
    }

    WINDOW* popup = createCenteredWindow(14, 68, 10, 42);
    if (!popup) {
        showTerminalSizeWarning(10, 42, hasColor);
        return -1;
    }
    keypad(popup, TRUE);
    int popupH = 0;
    int popupW = 0;
    getmaxyx(popup, popupH, popupW);
    const int contentW = std::max(1, popupW - 4);
    int selected = 0;
    while (true) {
        werase(popup);
        drawBoxSafe(popup);
        mvwprintw(popup, 1, 2, "Choose Sabotage Target");
        const int visibleRows = std::max(1, popupH - 5);
        for (size_t row = 0; row < candidates.size() && static_cast<int>(row) < visibleRows; ++row) {
            const int playerIndex = candidates[row];
            if (static_cast<int>(row) == selected) {
                wattron(popup, A_REVERSE);
            }
            const Player& target = players[static_cast<std::size_t>(playerIndex)];
            mvwprintw(popup, 3 + static_cast<int>(row), 2, "%s",
                      clipUiText(target.name + "  cash $" + std::to_string(target.cash) +
                                     "  worth $" + std::to_string(calculateFinalWorth(target)),
                                 static_cast<std::size_t>(contentW)).c_str());
            if (static_cast<int>(row) == selected) {
                wattroff(popup, A_REVERSE);
            }
        }
        mvwprintw(popup, popupH - 2, 2, "%s",
                  clipUiText("Up/Down move  ENTER select  ESC cancel",
                             static_cast<std::size_t>(contentW)).c_str());
        wrefresh(popup);

        const int ch = wgetch(popup);
        if (ch == KEY_UP) {
            selected = selected == 0 ? static_cast<int>(candidates.size()) - 1 : selected - 1;
        } else if (ch == KEY_DOWN) {
            selected = selected + 1 >= static_cast<int>(candidates.size()) ? 0 : selected + 1;
        } else if (isConfirmKey(ch)) {
            const int chosen = candidates[static_cast<std::size_t>(selected)];
            delwin(popup);
            return chosen;
        } else if (isCancelKey(ch)) {
            delwin(popup);
            return -1;
        }
    }
}

int Game::chooseTrapTile(int attackerIndex) {
    Player& player = players[static_cast<std::size_t>(attackerIndex)];
    std::string errorText;
    const int maxTrapTile = boardViewMode == BoardViewMode::Mode1860 ? board.tileCount() - 1 : TILE_COUNT - 1;
    while (true) {
        noecho();
        curs_set(1);
        werase(msgWin);
        drawBoxSafe(msgWin);
        mvwprintw(msgWin, 1, 2, "Trap tile id (0-%d) [%d]: ", maxTrapTile, player.tile);
        mvwprintw(msgWin, 2, 2, "Tip: choose a tile ahead of the leader. Blank uses your current tile.");
        if (!errorText.empty()) {
            mvwprintw(msgWin, 3, 2, "%s", errorText.c_str());
        }
        wrefresh(msgWin);

        char buffer[16] = {0};
        const bool confirmed = readLineOrCancel(msgWin, buffer, 15);
        noecho();
        curs_set(0);

        if (!confirmed) {
            return -1;
        }

        if (std::strlen(buffer) == 0) {
            return player.tile;
        }

        int tileId = -1;
        if (!parseStrictInt(buffer, tileId) ||
            tileId < 0 ||
            tileId > maxTrapTile ||
            (boardViewMode == BoardViewMode::Mode1860 && !board.isMode1860WalkableTile(tileId))) {
            errorText = "Invalid tile id. Enter a walkable tile from 0 to " + std::to_string(maxTrapTile) + ".";
            continue;
        }
        return tileId;
    }
}

bool Game::promptSabotageMenu(int attackerIndex) {
    Player& attacker = players[static_cast<std::size_t>(attackerIndex)];
    if (!isCpuPlayer(attackerIndex)) {
        maybeShowFirstTimeTutorial(TutorialTopic::Sabotage);
    }
    if (!isSabotageUnlockedForPlayer(attackerIndex)) {
        showInfoPopup("Sabotage locked",
                      "Sabotage unlocks on Turn " + std::to_string(std::max(1, settings.sabotageUnlockTurn)) + ".");
        return false;
    }
    WINDOW* popup = createCenteredWindow(19, 82, 19, 58);
    if (!popup) {
        showTerminalSizeWarning(19, 58, hasColor);
        return false;
    }
    keypad(popup, TRUE);
    int popupH = 0;
    int popupW = 0;
    getmaxyx(popup, popupH, popupW);
    const int contentW = std::max(1, popupW - 4);
    std::string status;

    while (true) {
        werase(popup);
        drawBoxSafe(popup);
        mvwprintw(popup, 1, 2, "%s",
                  clipUiText("Sabotage Menu - " + attacker.name, static_cast<std::size_t>(contentW)).c_str());
        mvwprintw(popup, 2, 2, "%s",
                  clipUiText("Cash $" + std::to_string(attacker.cash) +
                                 "  Shields " + std::to_string(attacker.shieldCards) +
                                 "  Insurance " + std::to_string(attacker.insuranceUses) +
                                 "  Cooldown " + std::to_string(attacker.sabotageCooldown),
                             static_cast<std::size_t>(contentW)).c_str());
        const std::vector<std::string> options{
            "1 Trap Tile ($12000)",
            "2 Lawsuit ($15000)",
            "3 Traffic Jam ($10000)",
            "4 Steal Action Card ($18000)",
            "5 Forced Duel Minigame ($22000)",
            "6 Career Sabotage ($24000)",
            "7 Position Swap ($90000, cooldown)",
            "8 Debt Trap ($20000)",
            "9 Buy Shield Card ($15000)",
            "0 Buy Insurance ($20000, 2 uses)",
            "D Item Disable ($16000)"
        };
        for (int i = 0; i < static_cast<int>(options.size()) && 4 + i < popupH - 4; ++i) {
            mvwprintw(popup, 4 + i, 2, "%s",
                      clipUiText(options[static_cast<std::size_t>(i)],
                                 static_cast<std::size_t>(contentW)).c_str());
        }
        if (!status.empty()) {
            mvwprintw(popup, popupH - 3, 2, "%s",
                      clipUiText(status, static_cast<std::size_t>(contentW)).c_str());
        }
        mvwprintw(popup, popupH - 2, 2, "%s",
                  clipUiText("Choose option or ESC cancel", static_cast<std::size_t>(contentW)).c_str());
        wrefresh(popup);

        const int ch = wgetch(popup);
        if (isCancelKey(ch)) {
            delwin(popup);
            return false;
        }
        if (ch == '9') {
            delwin(popup);
            PaymentResult payment = bank.charge(attacker, 15000);
            maybeShowLoanTutorial(attackerIndex, payment);
            ++attacker.shieldCards;
            maybeShowFirstTimeTutorial(TutorialTopic::Shield);
            std::string detail = "Shield Card added. Blocks one future sabotage.";
            if (payment.loansTaken > 0) {
                detail += " Automatic loans: " + std::to_string(payment.loansTaken) + ".";
            }
            showInfoPopup("Shield Card", detail);
            return true;
        }
        if (ch == '0') {
            delwin(popup);
            PaymentResult payment = bank.charge(attacker, 20000);
            maybeShowLoanTutorial(attackerIndex, payment);
            attacker.insuranceUses += 2;
            maybeShowFirstTimeTutorial(TutorialTopic::Insurance);
            std::string detail = "Insurance added. Next 2 money/property hits are halved.";
            if (payment.loansTaken > 0) {
                detail += " Automatic loans: " + std::to_string(payment.loansTaken) + ".";
            }
            showInfoPopup("Insurance", detail);
            return true;
        }
        if (ch == '1') {
            delwin(popup);
            const int tileId = chooseTrapTile(attackerIndex);
            if (tileId < 0) return false;
            const int trapChoice = showBranchPopup(
                "Choose Trap Effect",
                std::vector<std::string>{
                    "- Money loss: target pays cash",
                    "- Backward move: target is pushed back",
                    "- Skip turn: target loses their next turn",
                    "- Lose card: target drops an action card",
                    "- Minigame: target is forced into a duel"
                },
                'A',
                'B');
            if (trapChoice == MENU_CANCELLED) {
                return false;
            }
            SabotageType trapType = SabotageType::MoneyLoss;
            if (trapChoice == 1) trapType = SabotageType::MovementPenalty;
            else if (trapChoice == 2) trapType = SabotageType::SkipTurn;
            else if (trapChoice == 3) trapType = SabotageType::StealCard;
            else if (trapChoice == 4) trapType = SabotageType::ForceMinigame;
            placeTrap(attackerIndex, tileId, trapType);
            return true;
        }

        SabotageType type = SabotageType::MoneyLoss;
        if (ch == '2') type = SabotageType::MoneyLoss;
        else if (ch == '3') type = SabotageType::MovementPenalty;
        else if (ch == '4') type = SabotageType::StealCard;
        else if (ch == '5') type = SabotageType::ForceMinigame;
        else if (ch == '6') type = SabotageType::CareerPenalty;
        else if (ch == '7') type = SabotageType::PositionSwap;
        else if (ch == '8') type = SabotageType::DebtIncrease;
        else if (ch == 'd' || ch == 'D') type = SabotageType::ItemDisable;
        else {
            status = "Invalid choice. Use 0-9, D, or ESC.";
            continue;
        }

        delwin(popup);
        const int targetIndex = chooseSabotageTarget(attackerIndex);
        if (targetIndex >= 0) {
            executeSabotage(attackerIndex, targetIndex, type);
            return true;
        }
        return false;
    }
}

bool Game::setupPlayers() {
    noecho();
    curs_set(0);
    setupInProgress = true;
    clear();
    refresh();
    drawSetupTitle();
    int numPlayers = 0;
    std::vector<char> used;
    while (numPlayers < 2 || numPlayers > 4) {
        werase(msgWin);
        drawBoxSafe(msgWin);
        const int msgW = getmaxx(msgWin);
        const int contentW = std::max(1, msgW - 4);
        const std::string title = clipUiText("Players (2-4):", static_cast<std::size_t>(contentW));
        const std::string prompt = clipUiText("Enter number of players: ", static_cast<std::size_t>(contentW));
        mvwprintw(msgWin, 1, 2, "%s", title.c_str());
        mvwprintw(msgWin, 3, 2, "%s", prompt.c_str());
        refresh();
        wrefresh(msgWin);

        noecho();
        curs_set(1);
        char inputBuf[8] = {0};
        const bool confirmed = readLineOrCancel(msgWin, inputBuf, 7);
        noecho();
        curs_set(0);

        if (!confirmed) {
            return false;
        }

        if (inputBuf[0] >= '2' && inputBuf[0] <= '4' && inputBuf[1] == '\0') {
            numPlayers = inputBuf[0] - '0';
            break;
        }

        werase(msgWin);
        drawBoxSafe(msgWin);
        mvwprintw(msgWin, 1, 2, "%s",
                  clipUiText("Invalid input. Please enter 2, 3, or 4.",
                             static_cast<std::size_t>(contentW)).c_str());
        mvwprintw(msgWin, 2, 2, "%s",
                  clipUiText("Press any key to try again.", static_cast<std::size_t>(contentW)).c_str());
        refresh();
        wrefresh(msgWin);
        wgetch(msgWin);
        drawSetupTitle();
    }
    players.clear();
    players.reserve(numPlayers);
    for (int i = 0; i < numPlayers; ++i) {
        const int typeChoice = showBranchPopup(
            "Choose Player " + std::to_string(i + 1) + " of " + std::to_string(numPlayers) + " type",
            std::vector<std::string>{
                "- Human: this player uses keyboard input",
                "- CPU: computer-controlled opponent"
            },
            'A',
            'B');
        if (typeChoice == MENU_CANCELLED) {
            players.clear();
            return false;
        }
        PlayerType playerType = typeChoice == 0 ? PlayerType::Human : PlayerType::CPU;
        CpuDifficulty difficulty = CpuDifficulty::Normal;
        if (playerType == PlayerType::CPU) {
            const int difficultyChoice = showBranchPopup(
                "Choose CPU " + std::to_string(i + 1) + " difficulty",
                std::vector<std::string>{
                    "- Easy: more mistakes and simpler choices",
                    "- Normal: balanced decisions",
                    "- Hard: stronger money and route decisions"
                },
                'A',
                'B');
            if (difficultyChoice == MENU_CANCELLED) {
                players.clear();
                return false;
            }
            if (difficultyChoice == 0) {
                difficulty = CpuDifficulty::Easy;
            } else if (difficultyChoice == 2) {
                difficulty = CpuDifficulty::Hard;
            } else {
                difficulty = CpuDifficulty::Normal;
            }
        }

        drawSetupTitle();
        noecho();
        curs_set(1);
        werase(msgWin);
        drawBoxSafe(msgWin);
        const int msgW = getmaxx(msgWin);
        const int contentW = std::max(1, msgW - 4);
        const std::string setupTitle = clipUiText("Setting up player " + std::to_string(i + 1) +
                                                      " of " + std::to_string(numPlayers),
                                                  static_cast<std::size_t>(contentW));
        mvwprintw(msgWin, 1, 2, "%s", setupTitle.c_str());
        std::string namePrompt;
        if (playerType == PlayerType::CPU) {
            namePrompt = "CPU " + std::to_string(i + 1) +
                         " name [CPU " + std::to_string(i + 1) + "]: ";
        } else {
            namePrompt = "Player " + std::to_string(i + 1) + " name: ";
        }
        namePrompt = clipUiText(namePrompt, static_cast<std::size_t>(contentW));
        mvwprintw(msgWin, 3, 2, "%s", namePrompt.c_str());
        refresh();
        wrefresh(msgWin);
        char nameBuf[32] = {0};
        const bool nameConfirmed = readLineOrCancel(msgWin, nameBuf, 31);
        noecho();
        curs_set(0);

        if (!nameConfirmed) {
            players.clear();
            return false;
        }

        Player p;
        p.name = nameBuf;
        if (p.name.empty()) {
            p.name = playerType == PlayerType::CPU
                ? "CPU " + std::to_string(i + 1)
                : "Player " + std::to_string(i + 1);
        }
        p.token = showCharacterCustomisationPopup(used, p.name, i, playerType); //tokenForName(p.name, i);
        used.push_back(p.token);
        p.tile = boardViewMode == BoardViewMode::Mode1860 ? board.mode1860StartTileId() : 0;
        p.cash = settings.startingCash;
        p.job = "Unemployed";
        p.salary = 0;
        p.married = false;
        p.kids = 0;
        p.collegeGraduate = false;
        p.usedNightSchool = false;
        p.hasHouse = false;
        p.houseName = "";
        p.houseValue = 0;
        p.loans = 0;
        p.investedNumber = 0;
        p.investPayout = 0;
        p.spinToWinTokens = 0;
        p.retirementPlace = 0;
        p.retirementBonus = 0;
        p.finalHouseSaleValue = 0;
        p.retirementHome = "";
        p.actionCards.clear();
        p.petCards.clear();
        p.retired = false;
        p.startChoice = -1;
        p.familyChoice = -1;
        p.riskChoice = -1;
        p.type = playerType;
        p.cpuDifficulty = difficulty;
        p.sabotageDebt = 0;
        p.shieldCards = 0;
        p.insuranceUses = 0;
        p.skipNextTurn = false;
        p.movementPenaltyTurns = 0;
        p.movementPenaltyPercent = 0;
        p.salaryReductionTurns = 0;
        p.salaryReductionPercent = 0;
        p.sabotageCooldown = 0;
        p.itemDisableTurns = 0;
        p.hasFamilyPath = false;
        p.familyBabyEventsRemaining = 0;
        players.push_back(p);
        if (p.type == PlayerType::CPU) {
            addHistory("Joined CPU: " + p.name + " (" + cpuDifficultyLabel(p.cpuDifficulty) + ")");
        } else {
            addHistory("Joined: " + p.name);
        }
    }

    noecho();
    curs_set(0);
    return true;
}

char Game::showCharacterCustomisationPopup(const std::vector<char>& unavailableVec,const std::string& name, int index, PlayerType type) {
    std::vector<char> special = {
        '@', '#', '$', '%', '&', '*', '+', '=', '?', '!', '~', '^', '/'
    };

    std::vector<char> upper;
    std::vector<char> lower;

    for (char c = 'A'; c <= 'Z'; c++) upper.push_back(c);
    for (char c = 'a'; c <= 'z'; c++) lower.push_back(c);

    std::vector<char> symbols;
    symbols.insert(symbols.end(), special.begin(), special.end());
    symbols.insert(symbols.end(), upper.begin(), upper.end());
    symbols.insert(symbols.end(), lower.begin(), lower.end());

    std::set<char> unavailable(unavailableVec.begin(), unavailableVec.end());

    int selected = 0;

    const int cols = 13;
    const int cellW = 4;

    while (true) {
        clear();
        refresh();

        int screenH = 0;
        int screenW = 0;
        getmaxyx(stdscr, screenH, screenW);

        const int popupH = 27;
        const int popupW = 90;
        const int popupY = std::max(0, (screenH - popupH) / 2);
        const int popupX = std::max(0, (screenW - popupW) / 2);

        if (screenH < popupH || screenW < popupW) {
            showTerminalSizeWarning(popupH, popupW, hasColor);
            continue;
        }

        WINDOW* popup = newwin(popupH, popupW, popupY, popupX);
        if (!popup) {
            return '@';
        }

        keypad(popup, TRUE);
        apply_ui_background(popup);
        werase(popup);
        drawBoxSafe(popup);

        // ----- Title -----
        if (hasColor) {
            wattron(popup, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);
        }

        const std::string title = "CHARACTER CUSTOMISATION";
        const std::string subtitle = "Choose your trail marker";

        mvwprintw(popup, 1, (popupW - title.length()) / 2, "%s", title.c_str());
        mvwprintw(popup, 2, (popupW - subtitle.length()) / 2, "%s", subtitle.c_str());

        if (hasColor) {
            wattroff(popup, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);
        }

        // ----- Layout positions -----
        const int previewX = 5;
        const int previewY = 5;

        const int gridX = 28;
        const int gridY = 5;

        // ----- Preview panel -----
        const int boxW = 18;

        mvwprintw(popup, previewY, previewX, "+--------------+");

        std::string displayName = name.substr(0, boxW - 4);
        mvwprintw(popup, previewY + 1, previewX, "| %-12s |", displayName.c_str());

        wattron(popup, A_BOLD);
        mvwprintw(popup, previewY + 2, previewX, "|      %c       |", symbols[selected]);
        wattroff(popup, A_BOLD);

        std::string playerTypeText = (type == PlayerType::CPU) ? "CPU" : "HUMAN";
        std::string info = "P" + std::to_string(index + 1) +
                        " [" + std::string(1, symbols[selected]) + "] " +
                        playerTypeText;

        info = info.substr(0, boxW - 4);
        mvwprintw(popup, previewY + 3, previewX, "| %-12s |", info.c_str());

        mvwprintw(popup, previewY + 4, previewX, "+--------------+");

        if (hasColor) {
            wattroff(popup, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);
        }

        mvwprintw(popup, previewY + 7, previewX, "Status:");

        if (unavailable.count(symbols[selected]) > 0) {
            if (hasColor) {
                wattron(popup, COLOR_PAIR(GOLDRUSH_GOLD_TERRA) | A_BOLD);
            }

            mvwprintw(popup, previewY + 8, previewX, "Taken");

            if (hasColor) {
                wattroff(popup, COLOR_PAIR(GOLDRUSH_GOLD_TERRA) | A_BOLD);
            }
        } else {
            if (hasColor) {
                wattron(popup, COLOR_PAIR(GOLDRUSH_BLACK_FOREST) | A_BOLD);
            }

            mvwprintw(popup, previewY + 8, previewX, "Available");

            if (hasColor) {
                wattroff(popup, COLOR_PAIR(GOLDRUSH_BLACK_FOREST) | A_BOLD);
            }
        }

        // ----- Helper lambda to draw one section -----
        auto drawSection = [&](const std::string& title,
                               const std::vector<char>& section,
                               int startIndex,
                               int& y) {
            if (hasColor) {
                wattron(popup, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);
            }

            mvwprintw(popup, y, gridX, "%s", title.c_str());

            if (hasColor) {
                wattroff(popup, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);
            }

            y += 2;

            for (size_t i = 0; i < section.size(); i++) {
                int row = static_cast<int>(i) / cols;
                int col = static_cast<int>(i) % cols;

                int drawY = y + row * 2;
                int drawX = gridX + col * cellW;

                int globalIndex = startIndex + static_cast<int>(i);
                char current = symbols[globalIndex];

                bool taken = unavailable.count(current) > 0;
                bool cursor = globalIndex == selected;

                if (cursor) {
                    wattron(popup, A_REVERSE);
                }

                if (taken) {
                    wattron(popup, A_DIM);
                    mvwprintw(popup, drawY, drawX, "[%c]", current);
                    wattroff(popup, A_DIM);
                } else {
                    mvwprintw(popup, drawY, drawX, " %c ", current);
                }

                if (cursor) {
                    wattroff(popup, A_REVERSE);
                }
            }

            int rowsUsed = (static_cast<int>(section.size()) + cols - 1) / cols;
            y += rowsUsed * 2;
        };

        // ----- Draw grouped grid -----
        int y = gridY;

        drawSection("[ SYMBOLS ]", special, 0, y);

        drawSection("[ UPPERCASE ]",
                    upper,
                    static_cast<int>(special.size()),
                    y);

        drawSection("[ LOWERCASE ]",
                    lower,
                    static_cast<int>(special.size() + upper.size()),
                    y);

        // ----- Controls -----
        mvwprintw(popup, popupH - 5, 5, "Controls:");
        mvwprintw(popup, popupH - 4, 5, "Arrow keys / WASD  - move cursor");
        mvwprintw(popup, popupH - 3, 5, "ENTER              - choose symbol");
        mvwprintw(popup, popupH - 2, 5, "ESC                - cancel");

        wrefresh(popup);

        int ch = wgetch(popup);

        werase(popup);
        wrefresh(popup);
        delwin(popup);

        if (ch == KEY_RESIZE) {
            clear();
            touchwin(stdscr);
            refresh();
            continue;
        }

        if (ch == KEY_LEFT || ch == 'a' || ch == 'A') {
            selected = (selected - 1 + static_cast<int>(symbols.size())) %
                       static_cast<int>(symbols.size());
        } else if (ch == KEY_RIGHT || ch == 'd' || ch == 'D') {
            selected = (selected + 1) % static_cast<int>(symbols.size());
        } else if (ch == KEY_UP || ch == 'w' || ch == 'W') {
            selected = (selected - cols + static_cast<int>(symbols.size())) %
                       static_cast<int>(symbols.size());
        } else if (ch == KEY_DOWN || ch == 's' || ch == 'S') {
            selected = (selected + cols) % static_cast<int>(symbols.size());
        } else if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
            char chosen = symbols[selected];

            if (unavailable.count(chosen) == 0) {
                clear();
                touchwin(stdscr);
                refresh();
                return chosen;
            }
        } else if (ch == 27) {
            clear();
            touchwin(stdscr);
            refresh();
            return tokenForName(name,index);
        }
    }
}

int Game::showBranchPopup(const std::string& title,
                          const std::vector<std::string>& lines,
                          char a,
                          char b) {
    (void)a;
    (void)b;
    std::vector<int> values;
    for (std::size_t i = 0; i < lines.size(); ++i) {
        values.push_back(static_cast<int>(i));
    }

    if (setupInProgress && msgWin && !lines.empty()) {
        int highlighted = 0;
        while (true) {
            int msgH = 0;
            int msgW = 0;
            getmaxyx(msgWin, msgH, msgW);
            const int contentW = std::max(8, msgW - 4);
            werase(msgWin);
            drawBoxSafe(msgWin);
            mvwprintw(msgWin, 1, 2, "%s",
                      clipUiText(title, static_cast<std::size_t>(contentW)).c_str());
            for (std::size_t i = 0; i < lines.size() && 2 + static_cast<int>(i) < msgH - 1; ++i) {
                if (static_cast<int>(i) == highlighted) {
                    wattron(msgWin, A_REVERSE | A_BOLD);
                }
                mvwprintw(msgWin, 2 + static_cast<int>(i), 2, "%-*s",
                          contentW,
                          clipUiText(lines[i], static_cast<std::size_t>(contentW)).c_str());
                if (static_cast<int>(i) == highlighted) {
                    wattroff(msgWin, A_REVERSE | A_BOLD);
                }
            }
            wrefresh(msgWin);

            const int ch = wgetch(msgWin);
            //if (ch == KEY_RESIZE) {
                // Recreate the popup
            //    delwin(popup);
            //   popup = createCenteredWindow(popupH, popupW, startY, startX);
            //    if (!popup) return MENU_CANCELLED;
            //    keypad(popup, TRUE);
                // Redraw content
            //    continue;
            //}

            if (ch == KEY_RESIZE) {
                return MENU_CANCELLED;
            }
            if (ch == KEY_UP) {
                highlighted = highlighted == 0 ? static_cast<int>(lines.size()) - 1 : highlighted - 1;
            } else if (ch == KEY_DOWN) {
                highlighted = highlighted + 1 >= static_cast<int>(lines.size()) ? 0 : highlighted + 1;
            } else if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
                return values[static_cast<std::size_t>(highlighted)];
            } else if (ch == 27) {
                break;
            }
        }
    }

    if (!setupInProgress && boardWin && !lines.empty()) {
        auto clipToWidth = [](const std::string& text, int maxWidth) -> std::string {
            if (maxWidth <= 0) {
                return "";
            }
            if (static_cast<int>(text.size()) <= maxWidth) {
                return text;
            }
            return text.substr(0, static_cast<std::size_t>(maxWidth));
        };

        int boardH = 0;
        int boardW = 0;
        int boardY = 0;
        int boardX = 0;
        getmaxyx(boardWin, boardH, boardW);
        getbegyx(boardWin, boardY, boardX);

        int desiredWidth = static_cast<int>(title.size());
        for (std::size_t i = 0; i < lines.size(); ++i) {
            desiredWidth = std::max(desiredWidth, static_cast<int>(lines[i].size()));
        }
        const int popupW = std::max(24, std::min(boardW - 6, desiredWidth + 6));
        const int popupH = std::max(6, std::min(boardH - 4, static_cast<int>(lines.size()) + 4));
        const int popupY = boardY + std::max(1, boardH - popupH - 2);
        const int popupX = boardX + std::max(2, (boardW - popupW) / 2);

        WINDOW* popup = newwin(popupH, popupW, popupY, popupX);
        if (popup) {
            applyWindowBg(popup);
            keypad(popup, TRUE);
            int highlighted = 0;

            while (true) {
                werase(popup);
                drawBoxSafe(popup);
                const int contentW = std::max(1, popupW - 4);
                if (hasColor) wattron(popup, COLOR_PAIR(GOLDRUSH_GOLD_BLACK) | A_BOLD);
                mvwprintw(popup, 1, 2, "%s", clipToWidth(title, contentW).c_str());
                if (hasColor) wattroff(popup, COLOR_PAIR(GOLDRUSH_GOLD_BLACK) | A_BOLD);

                for (std::size_t i = 0; i < lines.size() && 2 + static_cast<int>(i) < popupH - 1; ++i) {
                    if (static_cast<int>(i) == highlighted) {
                        wattron(popup, A_REVERSE | A_BOLD);
                    }
                    mvwprintw(popup, 2 + static_cast<int>(i), 2, "%-*s",
                              contentW,
                              clipToWidth(lines[i], contentW).c_str());
                    if (static_cast<int>(i) == highlighted) {
                        wattroff(popup, A_REVERSE | A_BOLD);
                    }
                }
                wrefresh(popup);

                const int ch = wgetch(popup);
                if (ch == KEY_UP) {
                    highlighted = highlighted == 0 ? static_cast<int>(lines.size()) - 1 : highlighted - 1;
                } else if (ch == KEY_DOWN) {
                    highlighted = highlighted + 1 >= static_cast<int>(lines.size()) ? 0 : highlighted + 1;
                } else if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
                    delwin(popup);
                    touchwin(boardWin);
                    wrefresh(boardWin);
                    if (infoWin) {
                        touchwin(infoWin);
                        wrefresh(infoWin);
                    }
                    if (msgWin) {
                        touchwin(msgWin);
                        wrefresh(msgWin);
                    }
                    return values[static_cast<std::size_t>(highlighted)];
                } else if (ch == 27) {
                    break;
                }
            }

            delwin(popup);
            touchwin(boardWin);
            wrefresh(boardWin);
            if (infoWin) {
                touchwin(infoWin);
                wrefresh(infoWin);
            }
            if (msgWin) {
                touchwin(msgWin);
                wrefresh(msgWin);
            }
        }
    }

    return choose_branch_with_selector(title, lines, values, 0, 4);
}

int Game::showRequiredBranchPopup(const std::string& title,
                                  const std::vector<std::string>& lines,
                                  char a,
                                  char b) {
    while (true) {
        const int choice = showBranchPopup(title, lines, a, b);
        if (choice != MENU_CANCELLED) {
            return choice;
        }
        showInfoPopup("Choice required", "Choose an option to continue, or use the previous menu to go back.");
    }
}

int Game::playActionCard(int playerIndex, const Tile& tile) {
    Player& player = players[playerIndex];
    if (!isCpuPlayer(playerIndex)) {
        maybeShowFirstTimeTutorial(TutorialTopic::ActionCard);
    }
    ActionCard card;
    if (!decks.drawActionCard(card)) {
        showInfoPopup("Action Card", "No action cards are available.");
        return 0;
    }

    addHistory(player.name + " drew " + card.title);

    int amount = 0;
    int rollValue = 0;
    std::string branchText;
    std::string result;

    if (actionCardUsesRoll(card)) {
        rollValue = rollSpinner(card.title, "Hold SPACE to spin this card");
        addHistory(player.name + " rolled " + std::to_string(rollValue));

        const ActionRollOutcome* outcome = findMatchingRollOutcome(card, rollValue);
        if (outcome != 0) {
            branchText = outcome->text.empty()
                ? describeRollCondition(outcome->condition)
                : outcome->text;
            result = applyActionEffect(playerIndex, tile, outcome->effect, amount);
        } else {
            branchText = "No matching branch";
            result = "No effect.";
        }
    } else {
        result = applyActionEffect(playerIndex, tile, card.effect, amount);
    }

    if (card.keepAfterUse) {
        player.actionCards.push_back(card.title);
    }
    decks.resolveActionCard(card, card.keepAfterUse);
    if (branchText.empty()) {
        addHistory(player.name + " result: " + result);
    } else {
        addHistory(player.name + " result: " + branchText + " -> " + result);
    }

    WINDOW* popup = createCenteredWindow(12, 64, 12, 44);
    if (!popup) {
        showTerminalSizeWarning(12, 44, hasColor);
        return amount;
    }
    int popupH = 0;
    int popupW = 0;
    getmaxyx(popup, popupH, popupW);
    const int contentW = std::max(1, popupW - 4);
    werase(popup);
    drawBoxSafe(popup);
    mvwprintw(popup, 1, 2, "ACTION CARD");
    mvwprintw(popup, 2, 2, "%s", clipUiText(card.title, static_cast<std::size_t>(contentW)).c_str());
    mvwprintw(popup, 4, 2, "%s", clipUiText(card.description, static_cast<std::size_t>(contentW)).c_str());
    if (rollValue > 0) {
        mvwprintw(popup, 5, 2, "Rolled: %d", rollValue);
    }
    if (!branchText.empty()) {
        mvwprintw(popup, 6, 2, "%s",
                  clipUiText("Branch: " + branchText, static_cast<std::size_t>(contentW)).c_str());
    }
    if (amount > 0) {
        blinkIndicator(popup, 7, 2, clipUiText("Result: " + result, static_cast<std::size_t>(contentW)),
                       hasColor, GOLDRUSH_BLACK_FOREST, 2, 2000, contentW);
    } else if (amount < 0) {
        blinkIndicator(popup, 7, 2, clipUiText("Result: " + result, static_cast<std::size_t>(contentW)),
                       hasColor, GOLDRUSH_GOLD_TERRA, 2, 2000, contentW);
    } else {
        mvwprintw(popup, 7, 2, "%s",
                  clipUiText("Result: " + result, static_cast<std::size_t>(contentW)).c_str());
    }
    mvwprintw(popup, 8, 2, "%s",
              clipUiText(card.keepAfterUse ? "Kept for endgame scoring." : "Discarded after use.",
                         static_cast<std::size_t>(contentW)).c_str());
    mvwprintw(popup, popupH - 3, 2, "%s",
              clipUiText("Cash now: $" + std::to_string(player.cash) +
                             "  Loans: " + std::to_string(player.loans),
                         static_cast<std::size_t>(contentW)).c_str());
    mvwprintw(popup, popupH - 2, 2, "Press ENTER or ESC");
    wrefresh(popup);

    if (autoAdvanceUi) {
        napms(750);
    } else {
        int ch;
        do {
            ch = wgetch(popup);
        } while (!isConfirmKey(ch) && !isCancelKey(ch));
    }

    delwin(popup);
    touchwin(msgWin);
    wrefresh(msgWin);
    return amount;
}

