#pragma once

#include <ncurses.h>

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

enum class ControlScheme {
    SinglePlayer,
    DuelLeftPlayer,
    DuelRightPlayer
};

enum class MenuInputResult {
    None,
    Selected,
    Cancelled
};

const int MENU_CANCELLED = -1;

bool isCancelKey(int ch);
bool isConfirmKey(int ch, bool allowSpace = false);
bool waitForConfirmOrCancel(WINDOW* win, bool allowSpace = false);
bool readLineOrCancel(WINDOW* win, char* buffer, int maxLen);
MenuInputResult menuSelectOrCancel(int ch,
                                   int& selected,
                                   int optionCount,
                                   bool allowHorizontal = true,
                                   bool allowWasd = true,
                                   bool allowSpace = false);
InputAction getInputAction(int ch, ControlScheme scheme = ControlScheme::SinglePlayer);
