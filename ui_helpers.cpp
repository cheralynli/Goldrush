#include "ui_helpers.h"

#include "input_helpers.h"
#include "ui.h"

#include <algorithm>
#include <sstream>

namespace {
//Specify how long to blink when displaying "blinking" text
const int BLINK_HALF_MS = 160;

//Input: WINDOW win, int y(columns), int x(rows), string text, bool hasColor, int colorPair, int attrs
//Output: none
//Purpose: draws colored text in a specified window
//Relation: used in UI components
void drawSolidText(WINDOW* win,
                   int y,
                   int x,
                   const std::string& text,
                   bool hasColor,
                   int colorPair,
                   int attrs) {
    // Check bounds first
    if (!is_valid_draw_position(win, y, x)) {
        return;
    }

    if (hasColor) {
        wattron(win, COLOR_PAIR(colorPair) | attrs);
    } else {
        wattron(win, A_REVERSE | A_BOLD);
    }
    mvwprintw(win, y, x, "%s", text.c_str());
    if (hasColor) {
        wattroff(win, COLOR_PAIR(colorPair) | attrs);
    } else {
        wattroff(win, A_REVERSE | A_BOLD);
    }
}

//Input: WINDOW win, int y(columns), int x(rows), int width
//Output: none
//Purpose: clear text to simulate "blinking"
//Relation: used in blinkIndicator
void clearTextArea(WINDOW* win, int y, int x, int width) {
    if (width <= 0) {
        return;
    }
    if (!is_valid_draw_position(win, y, x)) {
        return;
    }
    safe_mvwprintw(win, y, x, "%-*s", width, "");
}

//Input: string text
//Output: vector splitLines
//Purpose: split text by '\n' into a vector of lines
//Relation: used by wrapUiText and into showPopupMessage
std::vector<std::string> splitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream in(text);
    std::string line;
    while (std::getline(in, line)) {
        lines.push_back(line);
    }
    if (lines.empty()) {
        lines.push_back("");
    }
    return lines;
}
}  // namespace

//Input: int minHeight, int minWidth
//Output: bool terminalIsAtLeast
//Purpose: check if terminal is correctly sized
//Relation: used in minigames to ensure game can be displayed properly
bool terminalIsAtLeast(int minHeight, int minWidth) {
    int h = 0;
    int w = 0;
    getmaxyx(stdscr, h, w);
    return h >= minHeight && w >= minWidth;
}

//Input: int minHeight, int minWidth, bool hasColor, bool waitForKey
//Output: none
//Purpose: display warning (terminal too small), and required/current terminal sizes
//Relation: used in minigames to ensure game can be displayed properly
void showTerminalSizeWarning(int minHeight, int minWidth, bool hasColor, bool waitForKey) {
    int h = 0;
    int w = 0;
    getmaxyx(stdscr, h, w);
    if (hasColor) {
        bkgd(COLOR_PAIR(GOLDRUSH_GOLD_BLACK));
    }
    clear();

    const std::string line1 = "Terminal too small - please resize";
    const std::string line2 = "Required: " + std::to_string(minWidth) + "x" + std::to_string(minHeight);
    const std::string line3 = "Current: " + std::to_string(w) + "x" + std::to_string(h);
    const std::string line4 = waitForKey ? "Press any key after resizing." : "Resize the terminal to continue.";
    const std::vector<std::string> lines{line1, line2, line3, line4};
    const int startY = std::max(0, (h - static_cast<int>(lines.size())) / 2);

    for (std::size_t i = 0; i < lines.size(); ++i) {
        const int x = std::max(0, (w - static_cast<int>(lines[i].size())) / 2);
        if (startY + static_cast<int>(i) >= 0 && startY + static_cast<int>(i) < h) {
            safe_mvwprintw(stdscr, startY + static_cast<int>(i), x, "%s", lines[i].c_str());
        }
    }
    refresh();

    if (waitForKey) {
        nodelay(stdscr, FALSE);
        getch();
    }
}

//Input: int preferredHeight, int preferredWidth, int minHeight, int minWidth
//Output: bool calculateCenteredWindowBounds
//Purpose: computes UiWindowBounds bounds based on preferences and constraints
//Relation: used in createCenteredWindow
bool calculateCenteredWindowBounds(int preferredHeight,
                                   int preferredWidth,
                                   int minHeight,
                                   int minWidth,
                                   UiWindowBounds& bounds) {
    int screenH = 0;
    int screenW = 0;
    getmaxyx(stdscr, screenH, screenW);
    minHeight = std::max(1, minHeight);
    minWidth = std::max(1, minWidth);
    if (screenH < minHeight || screenW < minWidth) {
        return false;
    }

    const int maxH = screenH >= minHeight + 2 ? screenH - 2 : screenH;
    const int maxW = screenW >= minWidth + 2 ? screenW - 2 : screenW;
    bounds.height = std::max(minHeight, std::min(preferredHeight, maxH));
    bounds.width = std::max(minWidth, std::min(preferredWidth, maxW));
    bounds.y = std::max(0, (screenH - bounds.height) / 2);
    bounds.x = std::max(0, (screenW - bounds.width) / 2);

    if (bounds.y + bounds.height > screenH) {
        bounds.y = std::max(0, screenH - bounds.height);
    }
    if (bounds.x + bounds.width > screenW) {
        bounds.x = std::max(0, screenW - bounds.width);
    }
    return bounds.height > 0 &&
           bounds.width > 0 &&
           bounds.y >= 0 &&
           bounds.x >= 0 &&
           bounds.y + bounds.height <= screenH &&
           bounds.x + bounds.width <= screenW;
}

//Input: int preferredHeight, int preferredWidth, int minHeight, int minWidth
//Output: WINDOW createCenteredWindow
//Purpose: create window for popup messages
//Relation: used in showPopupMessage
WINDOW* createCenteredWindow(int preferredHeight, int preferredWidth, int minHeight, int minWidth) {
    UiWindowBounds bounds{0, 0, 0, 0};
    if (!calculateCenteredWindowBounds(preferredHeight, preferredWidth, minHeight, minWidth, bounds)) {
        return nullptr;
    }
    WINDOW* win = newwin(bounds.height, bounds.width, bounds.y, bounds.x);
    if (win) {
        apply_ui_background(win);
    }
    return win;
}

//Input: WINDOW win
//Output: none
//Purpose: clears area within a bordered window (row 1 to row h-2)
//Relation: 
void clearWindowInterior(WINDOW* win) {
    if (!win) {
        return;
    }
    int h = 0;
    int w = 0;
    getmaxyx(win, h, w);
    for (int y = 1; y < h - 1; ++y) {
        mvwprintw(win, y, 1, "%*s", std::max(0, w - 2), "");
    }
}

//Input: WINDOW win, int y(columns), int x(rows), string message
//Output: none
//Purpose: wait for user to enter prompt
//Relation: used in showPopupMessage while waiting for prompt
void waitForEnterPrompt(WINDOW* win, int y, int x, const std::string& message) {
    if (!win) {
        return;
    }
    if (!message.empty()) {
        safe_mvwprintw(win, y, x, "%s", message.c_str());
        wrefresh(win);
    }

    nodelay(win, FALSE);
    waitForConfirmOrCancel(win);
}

//Input: string text, int blinkCount, int finalHoldMs
//Output: blinkIndicator()
//Purpose: simplify blinkIndicator()
//Relation: convenience wrapper for blinkIndicator()
void blinkIndicator(const std::string& text, int blinkCount, int finalHoldMs) {
    int h = 0;
    int w = 0;
    getmaxyx(stdscr, h, w);
    const int x = std::max(0, (w - static_cast<int>(text.size())) / 2);
    const int y = std::max(0, h / 2);
    blinkIndicator(stdscr,
                   y,
                   x,
                   text,
                   has_colors(),
                   GOLDRUSH_GOLD_SAND,
                   blinkCount,
                   finalHoldMs,
                   static_cast<int>(text.size()));
}

//Input: WINDOW win, int y(columns), int x(rows), string text, bool hasColor, int colorPair, int blinkCount, int finalHoldMs, int clearWidth
//Output: none
//Purpose: draw blinking text that blinks blinkCount times
//Relation: used in game.cpp prompts
void blinkIndicator(WINDOW* win,
                    int y,
                    int x,
                    const std::string& text,
                    bool hasColor,
                    int colorPair,
                    int blinkCount,
                    int finalHoldMs,
                    int clearWidth) {
    if (!win || text.empty()) {
        return;
    }

    const int width = clearWidth > 0 ? clearWidth : static_cast<int>(text.size());
    drawSolidText(win, y, x, text, hasColor, colorPair, A_BOLD);
    wrefresh(win);
    napms(BLINK_HALF_MS);

    for (int blink = 0; blink < blinkCount; ++blink) {
        clearTextArea(win, y, x, width);
        wrefresh(win);
        napms(BLINK_HALF_MS);

        drawSolidText(win, y, x, text, hasColor, colorPair, A_BOLD | A_BLINK);
        wrefresh(win);
        napms(BLINK_HALF_MS);
    }

    clearTextArea(win, y, x, width);
    drawSolidText(win, y, x, text, hasColor, colorPair, A_BOLD);
    wrefresh(win);
    if (finalHoldMs > 0) {
        napms(finalHoldMs);
    }
}

//Input: string text, size_t width
//Output: wrapped vector<string> wrapUiText
//Purpose: wrap text according to width
//Relation: used in showPopupMessage to ensure text fits in popup windows
std::vector<std::string> wrapUiText(const std::string& text, std::size_t width) {
    if (width == 0) {
        return splitLines(text);
    }

    std::vector<std::string> out;
    const std::vector<std::string> rawLines = splitLines(text);
    for (std::size_t raw = 0; raw < rawLines.size(); ++raw) {
        std::istringstream words(rawLines[raw]);
        std::string word;
        std::string line;
        while (words >> word) {
            if (word.size() > width) {
                if (!line.empty()) {
                    out.push_back(line);
                    line.clear();
                }
                out.push_back(word.substr(0, width));
                continue;
            }

            if (line.empty()) {
                line = word;
            } else if (line.size() + 1 + word.size() <= width) {
                line += " " + word;
            } else {
                out.push_back(line);
                line = word;
            }
        }
        if (!line.empty()) {
            out.push_back(line);
        }
        if (rawLines[raw].empty()) {
            out.push_back("");
        }
    }
    if (out.empty()) {
        out.push_back("");
    }
    return out;
}

//Input: string text, size_t width
//Output: clipped string clipUiText
//Purpose: clip text to width-3 if text is too long
//Relation: used in showPopupMessage to ensure text fits in popup windows
std::string clipUiText(const std::string& text, std::size_t width) {
    if (text.size() <= width) {
        return text;
    }
    if (width <= 3) {
        return text.substr(0, width);
    }
    return text.substr(0, width - 3) + "...";
}

//Input: string title, string message, bool hasColor, bool autoAdvance
//Output: showPopupMessage()
//Purpose: splits message into a vector of strings
//Relation: pre-processing of message before entering showPopupMessage
void showPopupMessage(const std::string& title,
                      const std::string& message,
                      bool hasColor,
                      bool autoAdvance) {
    showPopupMessage(title, splitLines(message), hasColor, autoAdvance);
}

//Input: string title, vector lines, bool hasColor, bool autoAdvance
//Output: none
//Purpose: create a window to display popup message
//Relation: used in showDecisionPopup and in game.cpp
void showPopupMessage(const std::string& title,
                      const std::vector<std::string>& lines,
                      bool hasColor,
                      bool autoAdvance) {
    int screenH = 0;
    int screenW = 0;
    getmaxyx(stdscr, screenH, screenW);

    const int popupW = std::min(std::max(58, screenW - 8), 86);
    const int estimatedContentW = std::max(1, popupW - 6);

    std::vector<std::string> wrapped;
    for (std::size_t i = 0; i < lines.size(); ++i) {
        const std::vector<std::string> part =
            wrapUiText(lines[i], static_cast<std::size_t>(estimatedContentW));
        wrapped.insert(wrapped.end(), part.begin(), part.end());
    }

    const int preferredH = std::max(15, static_cast<int>(wrapped.size()) + 11);
    const int popupH = std::min(preferredH, std::max(8, screenH - 2));

    WINDOW* popup = createCenteredWindow(popupH, popupW, 8, 32);
    if (!popup) {
        showTerminalSizeWarning(8, 32, hasColor, !autoAdvance);
        return;
    }

    int actualH = 0;
    int actualW = 0;
    getmaxyx(popup, actualH, actualW);

    const int actualContentW = std::max(1, actualW - 6);

    wrapped.clear();
    for (std::size_t i = 0; i < lines.size(); ++i) {
        const std::vector<std::string> part =
            wrapUiText(lines[i], static_cast<std::size_t>(actualContentW));
        wrapped.insert(wrapped.end(), part.begin(), part.end());
    }

    keypad(popup, TRUE);
    werase(popup);
    drawBoxSafe(popup);

    // Title row: blinking is applied directly here, not one row below it.
    const int titleY = 1;
    const int titleX = 3;
    const std::string titleText =
        clipUiText(title, static_cast<std::size_t>(actualContentW));

    blinkIndicator(popup,
                   titleY,
                   titleX,
                   titleText,
                   hasColor,
                   GOLDRUSH_GOLD_SAND,
                   2,
                   0,
                   actualContentW);

    const int bodyStartY = 4;

    // Bottom usable row, same visual placement as the turn summary popup.
    const int promptY = std::max(1, actualH - 2);

    // Keep prompt safely inside the border and prevent wrapping.
    const int promptX = std::min(5, std::max(1, actualW - 2));

    // Keep one blank row above the prompt.
    const int blankRowY = promptY - 1;
    const int maxBodyLines = std::max(0, blankRowY - bodyStartY);

    const bool truncated = static_cast<int>(wrapped.size()) > maxBodyLines;
    const int linesToDraw =
        truncated && maxBodyLines > 0 ? maxBodyLines - 1 : maxBodyLines;

    for (int i = 0; i < linesToDraw && i < static_cast<int>(wrapped.size()); ++i) {
        mvwprintw(popup,
                  bodyStartY + i,
                  3,
                  "%s",
                  clipUiText(wrapped[static_cast<std::size_t>(i)],
                             static_cast<std::size_t>(actualContentW)).c_str());
    }

    if (truncated && maxBodyLines > 0) {
        mvwprintw(popup,
                  bodyStartY + maxBodyLines - 1,
                  3,
                  "%s",
                  clipUiText("...more text omitted...",
                             static_cast<std::size_t>(actualContentW)).c_str());
    }

    // Clear interior rows without touching the border.
    if (blankRowY > 0) {
        for (int x = 1; x < actualW - 1; ++x) {
            mvwaddch(popup, blankRowY, x, ' ');
        }
    }
    for (int x = 1; x < actualW - 1; ++x) {
        mvwaddch(popup, promptY, x, ' ');
    }

    const std::string promptText = autoAdvance
        ? "Continuing..."
        : "Press ENTER or ESC to continue...";

    // Use addnstr instead of printw so the prompt cannot wrap.
    const int maxPromptWidth = std::max(1, actualW - promptX - 2);
    const std::string clippedPrompt =
        clipUiText(promptText, static_cast<std::size_t>(maxPromptWidth));

    mvwaddnstr(popup,
               promptY,
               promptX,
               clippedPrompt.c_str(),
               maxPromptWidth);

    wrefresh(popup);

    if (autoAdvance) {
        napms(2000);
    } else {
        nodelay(popup, FALSE);
        waitForConfirmOrCancel(popup);
    }

    delwin(popup);
    wnoutrefresh(stdscr);
    doupdate();
}

//Input: string playerName, string decision, string explanation
//Output: showPopupMessage()
//Purpose: pop up a decision window
//Relation: used in game.cpp
void showDecisionPopup(const std::string& playerName,
                       const std::string& decision,
                       const std::string& explanation,
                       bool hasColor,
                       bool autoAdvance) {
    std::vector<std::string> lines;
    lines.push_back(playerName + " chooses " + decision + ".");
    lines.push_back("");
    lines.push_back(explanation);
    showPopupMessage("Decision Confirmed", lines, hasColor, autoAdvance);
}


// ============================================
// Safe drawing functions with bounds checking
// ============================================

bool is_valid_draw_position(WINDOW* win, int y, int x) {
    if (!win) return false;
    int max_y = getmaxy(win);
    int max_x = getmaxx(win);
    return (y >= 0 && y < max_y && x >= 0 && x < max_x);
}

void safe_mvwprintw(WINDOW* win, int y, int x, const char* fmt, ...) {
    if (!win || !fmt) return;
    
    // Check bounds before drawing
    if (!is_valid_draw_position(win, y, x)) {
        // Silently ignore - don't crash
        return;
    }
    
    va_list args;
    va_start(args, fmt);
    vw_printw(win, fmt, args);
    va_end(args);
}

void safe_mvwaddch(WINDOW* win, int y, int x, chtype ch) {
    if (!win) return;
    if (!is_valid_draw_position(win, y, x)) {
        return;
    }
    mvwaddch(win, y, x, ch);
}

void safe_mvwaddstr(WINDOW* win, int y, int x, const char* str) {
    if (!win || !str) return;
    if (!is_valid_draw_position(win, y, x)) {
        return;
    }
    mvwaddstr(win, y, x, str);
}

void safe_addch(WINDOW* win, int y, int x, chtype ch) {
    safe_mvwaddch(win, y, x, ch);
}
