#pragma once

#include <ncurses.h>

//Purpose: defines possible player actions derived from key input
//Relation: used in minigames (Pong, Battleship, etc.) to interpret keystrokes into gameplay actions.
enum class InputAction {
    Up,
    Down,
    Left,
    Right,
    Confirm,
    Cancel,
    Start,
    Reload,
    Fire,
    None
};

//Purpose: defines input mapping depending on game mode (solo or duel)
//Relation: passed into getInputAction to interpret keys correctly for each player.
enum class ControlScheme {
    SinglePlayer,
    DuelLeftPlayer,
    DuelRightPlayer
};

//Purpose: defines possible outcomes of menu navigation
//Relation: used in menus to handle selection or cancellation.
enum class MenuInputResult {
    None,
    Selected,
    Cancelled
};

//Purpose: sentinel value for cancelled menu actions
//Relation: used in menu handling functions.
const int MENU_CANCELLED = -1;

//Input: key code
//Output: true if key corresponds to cancel action
//Purpose: detects cancel input (e.g., ESC)
//Relation: used in menus and minigames to exit early.
bool isCancelKey(int ch);

//Input: key code, optional flag to allow spacebar
//Output: true if key corresponds to confirm action
//Purpose: detects confirm input (e.g., Enter, Space if allowed)
//Relation: used in menus and minigames for confirmation.
bool isConfirmKey(int ch, bool allowSpace = false);

//Input: ncurses window, optional spacebar flag
//Output: true if confirm pressed, false if cancel pressed
//Purpose: blocks until user confirms or cancels
//Relation: used in tutorials or prompts.
bool waitForConfirmOrCancel(WINDOW* win, bool allowSpace = false);

//Input: ncurses window, buffer pointer, max length
//Output: true if line read successfully, false if cancelled
//Purpose: reads text input from user with cancel option
//Relation: used for name entry or text prompts.
bool readLineOrCancel(WINDOW* win, char* buffer, int maxLen);

//Input: key code, reference to selected index, option count, flags for input styles
//Output: MenuInputResult (None, Selected, Cancelled)
//Purpose: interprets menu navigation input
//Relation: used in menus to move selection and confirm/cancel.
MenuInputResult menuSelectOrCancel(int ch,
                                   int& selected,
                                   int optionCount,
                                   bool allowHorizontal = true,
                                   bool allowWasd = true,
                                   bool allowSpace = false);

//Input: key code, control scheme
//Output: InputAction enum value
//Purpose: maps raw key input to gameplay action depending on scheme
//Relation: core function for translating keystrokes into actions in minigames.
InputAction getInputAction(int ch, ControlScheme scheme = ControlScheme::SinglePlayer);
