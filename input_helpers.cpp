#include "input_helpers.h"

#include <cctype>
#include <cstring>

#include <ncurses.h>

//Input: integer key code
//Output: true if key is ESC (27)
//Purpose: detects cancel/exit input
//Relation: used across menus and minigames to allow early exit.
bool isCancelKey(int ch) {
    return ch == 27;
}

//Input: integer key code, flag to allow spacebar
//Output: true if Enter/Return/KEY_ENTER pressed, or space if allowed
//Purpose: detects confirmation input
//Relation: used in menus and minigames for confirming actions.
bool isConfirmKey(int ch, bool allowSpace) {
    return ch == '\n' || ch == '\r' || ch == KEY_ENTER || (allowSpace && ch == ' ');
}

//Input: ncurses window, optional spacebar flag
//Output: true if confirm pressed, false if cancel pressed
//Purpose: blocks until user confirms or cancels
//Relation: used in tutorials or prompts where explicit user choice is required.
bool waitForConfirmOrCancel(WINDOW* win, bool allowSpace) {
    if (!win) {
        return false;
    }

    while (true) {
        const int ch = wgetch(win);
        if (isCancelKey(ch)) {
            return false;
        }
        if (isConfirmKey(ch, allowSpace)) {
            return true;
        }
    }
}

//Input: ncurses window, buffer pointer, maximum length
//Output: true if line read successfully, false if cancelled
//Purpose: reads user text input with cancel and backspace handling
//Relation: used for name entry or text prompts in menus.
bool readLineOrCancel(WINDOW* win, char* buffer, int maxLen) {
    if (!win || !buffer || maxLen <= 0) {
        return false;
    }

    buffer[0] = '\0';
    int y = 0;
    int startX = 0;
    getyx(win, y, startX);

    int length = 0;
    while (true) {
        const int ch = wgetch(win);
        if (isCancelKey(ch)) {
            buffer[0] = '\0';
            return false;
        }
        if (isConfirmKey(ch)) {
            buffer[length] = '\0';
            return true;
        }
        if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
            if (length > 0) {
                --length;
                buffer[length] = '\0';
                mvwaddch(win, y, startX + length, ' ');
                wmove(win, y, startX + length);
                wrefresh(win);
            }
            continue;
        }
        if (length >= maxLen) {
            continue;
        }
        if (ch >= 0 && ch <= 255 && std::isprint(static_cast<unsigned char>(ch))) {
            buffer[length] = static_cast<char>(ch);
            ++length;
            buffer[length] = '\0';
            waddch(win, ch);
            wrefresh(win);
        }
    }
}

//Input: key code, reference to selected index, option count, flags for input styles
//Output: MenuInputResult (None, Selected, Cancelled)
//Purpose: interprets menu navigation input (up/down/left/right, WASD, confirm/cancel)
//Relation: used in menus to move selection and confirm/cancel choices.
MenuInputResult menuSelectOrCancel(int ch,
                                   int& selected,
                                   int optionCount,
                                   bool allowHorizontal,
                                   bool allowWasd,
                                   bool allowSpace) {
    if (isCancelKey(ch)) {
        return MenuInputResult::Cancelled;
    }
    if (optionCount <= 0) {
        return MenuInputResult::None;
    }
    if (isConfirmKey(ch, allowSpace)) {
        return MenuInputResult::Selected;
    }

    const bool up = ch == KEY_UP ||
        (allowHorizontal && ch == KEY_LEFT) ||
        (allowWasd && (ch == 'w' || ch == 'W' || ch == 'a' || ch == 'A'));
    const bool down = ch == KEY_DOWN ||
        (allowHorizontal && ch == KEY_RIGHT) ||
        (allowWasd && (ch == 's' || ch == 'S' || ch == 'd' || ch == 'D'));

    if (up) {
        selected = selected == 0 ? optionCount - 1 : selected - 1;
        return MenuInputResult::None;
    }
    if (down) {
        selected = selected + 1 >= optionCount ? 0 : selected + 1;
        return MenuInputResult::None;
    }
    return MenuInputResult::None;
}

//Input: key code, control scheme (SinglePlayer, DuelLeftPlayer, DuelRightPlayer)
//Output: InputAction enum (Up, Down, Left, Right, Confirm, Cancel, Start, Reload, Fire, None)
//Purpose: maps raw key input to gameplay actions depending on scheme
//Relation: core function for translating keystrokes into actions in minigames (Pong, Battleship, etc.).
InputAction getInputAction(int ch, ControlScheme scheme) {
    if (isCancelKey(ch)) {
        return InputAction::Cancel;
    }

    if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
        return InputAction::Confirm;
    }
    if (ch == ' ') {
        return InputAction::Fire;
    }
    if (ch == 'x' || ch == 'X') {
        return InputAction::Start;
    }
    if (ch == 'r' || ch == 'R') {
        return InputAction::Reload;
    }

    if (scheme == ControlScheme::DuelRightPlayer) {
        if (ch == KEY_UP) return InputAction::Up;
        if (ch == KEY_DOWN) return InputAction::Down;
        if (ch == KEY_LEFT) return InputAction::Left;
        if (ch == KEY_RIGHT) return InputAction::Right;
        return InputAction::None;
    }

    if (scheme == ControlScheme::DuelLeftPlayer) {
        if (ch == 'w' || ch == 'W') return InputAction::Up;
        if (ch == 's' || ch == 'S') return InputAction::Down;
        if (ch == 'a' || ch == 'A') return InputAction::Left;
        if (ch == 'd' || ch == 'D') return InputAction::Right;
        return InputAction::None;
    }

    if (ch == KEY_UP || ch == 'w' || ch == 'W') return InputAction::Up;
    if (ch == KEY_DOWN || ch == 's' || ch == 'S') return InputAction::Down;
    if (ch == KEY_LEFT || ch == 'a' || ch == 'A') return InputAction::Left;
    if (ch == KEY_RIGHT || ch == 'd' || ch == 'D') return InputAction::Right;
    return InputAction::None;
}
