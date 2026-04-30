#include "ui_layout.h"

#include <algorithm>

#include <ncurses.h>

namespace {
//dimensions for full vs compact sized games
const int FULL_HEADER_HEIGHT = 9;
const int FULL_BOARD_WIDTH = 82;
const int FULL_BOARD_HEIGHT = 31;
const int FULL_SIDE_PANEL_WIDTH = 42;
const int FULL_MESSAGE_HEIGHT = 6;

const int COMPACT_HEADER_HEIGHT = 4;
const int COMPACT_BOARD_WIDTH = 82;
const int COMPACT_BOARD_HEIGHT = 29;
const int COMPACT_SIDE_PANEL_WIDTH = 34;
const int COMPACT_MESSAGE_HEIGHT = 5;
}

//Input: int termHeight, int termWidth (terminal height/width)
//Output: UILayout calculateUILayout
//Purpose: determine UI size based on size of terminal
//Relation: used in createWindows and chooseBoardViewMode in game.cpp
UILayout calculateUILayout(int termHeight, int termWidth) {
    if (termHeight < 0 || termWidth < 0) {
        getmaxyx(stdscr, termHeight, termWidth);
    }

    const int fullHeight = FULL_HEADER_HEIGHT + FULL_BOARD_HEIGHT + FULL_MESSAGE_HEIGHT;
    const bool useCompact = termHeight < fullHeight;

    UILayout layout;
    layout.compact = useCompact;
    layout.headerHeight = useCompact ? COMPACT_HEADER_HEIGHT : FULL_HEADER_HEIGHT;
    layout.boardWidth = useCompact ? COMPACT_BOARD_WIDTH : FULL_BOARD_WIDTH;
    layout.boardHeight = useCompact ? COMPACT_BOARD_HEIGHT : FULL_BOARD_HEIGHT;
    layout.sidePanelWidth = useCompact ? COMPACT_SIDE_PANEL_WIDTH : FULL_SIDE_PANEL_WIDTH;
    layout.sidePanelHeight = layout.boardHeight;
    layout.messageHeight = useCompact ? COMPACT_MESSAGE_HEIGHT : FULL_MESSAGE_HEIGHT;
    layout.totalWidth = layout.boardWidth + layout.sidePanelWidth;
    layout.totalHeight = layout.headerHeight + layout.boardHeight + layout.messageHeight;
    layout.originX = std::max(0, (termWidth - layout.totalWidth) / 2);
    layout.originY = (termHeight > layout.totalHeight) ? 1 : 0;
    return layout;
}

//Input: none
//Output: int minimumGameWidth
//Purpose: minimum terminal width to run game
//Relation: used in ensureMinSize
int minimumGameWidth() {
    return 124;
}

//Input: none
//Output: int minimumGameHeight
//Purpose: minimum terminal height to run game
//Relation: used in ensureMinSize
int minimumGameHeight() {
    return 45;
}
