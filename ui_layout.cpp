#include "ui_layout.h"
#include "board.hpp"

#include <algorithm>

#include <ncurses.h>

namespace {
const int FULL_HEADER_HEIGHT = 4;
const int FULL_SIDE_PANEL_WIDTH = 42;
const int FULL_MESSAGE_HEIGHT = 5;

const int COMPACT_HEADER_HEIGHT = 4;
const int COMPACT_SIDE_PANEL_WIDTH = 42;
const int COMPACT_MESSAGE_HEIGHT = 5;
}

UILayout calculateUILayout(int termHeight, int termWidth) {
    if (termHeight < 0 || termWidth < 0) {
        getmaxyx(stdscr, termHeight, termWidth);
    }

    const int boardGridWidth = BOARD_COLS * CELL_W + 1;
    const int boardGridHeight = BOARD_ROWS * CELL_H + 1;
    const int boardPanelWidth = boardGridWidth + (BOARD_PAD_X * 2) + 2;
    const int boardPanelHeight = boardGridHeight + (BOARD_PAD_Y * 2) + 2;
    const int fullHeight = FULL_HEADER_HEIGHT + boardPanelHeight + FULL_MESSAGE_HEIGHT;
    const bool useCompact = termHeight < fullHeight;

    UILayout layout;
    layout.compact = useCompact;
    layout.headerHeight = useCompact ? COMPACT_HEADER_HEIGHT : FULL_HEADER_HEIGHT;
    layout.boardWidth = boardPanelWidth;
    layout.boardHeight = boardPanelHeight;
    layout.sidePanelWidth = useCompact ? COMPACT_SIDE_PANEL_WIDTH : FULL_SIDE_PANEL_WIDTH;
    layout.sidePanelHeight = layout.boardHeight;
    layout.messageHeight = useCompact ? COMPACT_MESSAGE_HEIGHT : FULL_MESSAGE_HEIGHT;
    layout.totalWidth = layout.boardWidth + layout.sidePanelWidth;
    layout.totalHeight = layout.headerHeight + layout.boardHeight + layout.messageHeight;
    layout.originX = std::max(0, (termWidth - layout.totalWidth) / 2);
    layout.originY = (termHeight > layout.totalHeight) ? 1 : 0;
    return layout;
}

int minimumGameWidth() {
    const int boardGridWidth = BOARD_COLS * CELL_W + 1;
    const int boardPanelWidth = boardGridWidth + (BOARD_PAD_X * 2) + 2;
    return boardPanelWidth + FULL_SIDE_PANEL_WIDTH;
}

int minimumGameHeight() {
    const int boardGridHeight = BOARD_ROWS * CELL_H + 1;
    const int boardPanelHeight = boardGridHeight + (BOARD_PAD_Y * 2) + 2;
    return FULL_HEADER_HEIGHT + boardPanelHeight + FULL_MESSAGE_HEIGHT;
}
