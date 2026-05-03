#include "board.hpp"
#include "tile_display.h"
#include "ui.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <set>
#include <utility>
#include <string>
#include <vector>

namespace {

//Input: none
//Output: x/y coordinate pair
//Purpose: stores a classic board drawing coordinate
//Relation: used by CLASSIC_BOARD_POSITIONS
struct UiPos {
    int x;
    int y;
};

static const UiPos CLASSIC_BOARD_POSITIONS[TILE_COUNT] = {
    {17, 1}, {21, 1}, {25, 1}, {29, 1}, {33, 1}, {37, 1}, {41, 1}, {45, 1}, {49, 1}, {53, 1}, {57, 1}, {61, 1}, {39, 4},
    {3, 7}, {7, 7}, {11, 7}, {15, 7}, {19, 7}, {23, 7}, {27, 7}, {31, 7}, {35, 7}, {39, 7}, {43, 7}, {47, 7},
    {25, 10}, {29, 10}, {33, 10}, {37, 10}, {41, 10}, {45, 10}, {49, 10}, {53, 10}, {57, 10}, {61, 10}, {65, 10}, {69, 10}, {73, 10},
    {19, 13}, {23, 13}, {27, 13}, {31, 13}, {35, 13}, {39, 13}, {43, 13}, {47, 13}, {51, 13}, {55, 13}, {59, 13},
    {21, 16}, {25, 16}, {29, 16}, {33, 16}, {37, 16}, {41, 16}, {45, 16}, {49, 16}, {53, 16}, {57, 16},
    {3, 19}, {7, 19}, {11, 19}, {15, 19}, {19, 19}, {23, 19}, {27, 19}, {31, 19}, {35, 19}, {39, 19}, {43, 19}, {47, 19}, {51, 19}, {55, 19},
    {23, 22}, {27, 22}, {31, 22}, {35, 22}, {39, 22}, {43, 22}, {47, 22}, {51, 22}, {55, 22}, {59, 22}, {63, 22}, {67, 22}, {71, 22}, {75, 22},
    {37, 25}, {41, 25}
};

//Input: none
//Output: classic row segment metadata
//Purpose: groups contiguous classic board tiles by row for route drawing
//Relation: used by drawClassicBoardGrid
struct RowSegment {
    int y;
    int startIndex;
    int count;
};

static const RowSegment CLASSIC_ROW_SEGMENTS[] = {
    {1, 0, 12},
    {4, 12, 1},
    {7, 13, 12},
    {10, 25, 13},
    {13, 38, 11},
    {16, 49, 10},
    {19, 59, 14},
    {22, 73, 14},
    {25, 87, 2}
};

//Input: none
//Output: classic region label metadata
//Purpose: stores text and position for region names on the classic board
//Relation: used by drawClassicRegions
struct RegionLabel {
    const char* name;
    int y;
    int x;
};

static const RegionLabel CLASSIC_REGION_LABELS[] = {
    {"STARTUP STREET", 2, 28},
    {"CAREER CITY", 5, 28},
    {"GOLDRUSH VALLEY", 11, 27},
    {"FAMILY AVENUE", 17, 24},
    {"RISKY ROAD", 23, 28},
    {"RETIREMENT RIDGE", 26, 25}
};

const int PLAYER_PAIR_BASE = 9;
const int VIEW_COLS = 5;
const int VIEW_ROWS = 3;
const int TILE_W = 12;
const int TILE_H = 5;
const int TILE_GAP_X = 1;
const int TILE_GAP_Y = 1;
const int MODE1860_ROWS = 25;
const int MODE1860_COLS = 25;
const int MODE1860_CELL_W = 6;
const int MODE1860_CELL_H = 2;
const int MODE1860_BASE_TILE_ID = TILE_COUNT;
const short MODE1860_PAIR_CHECKER_LIGHT = 24;
const short MODE1860_PAIR_CHECKER_DARK = 25;
const short MODE1860_PAIR_TILE_ROUTE = 26;
const short MODE1860_PAIR_TILE_PAYDAY = 27;
const short MODE1860_PAIR_TILE_ACTION = 28;
const short MODE1860_PAIR_TILE_MINIGAME = 29;
const short MODE1860_PAIR_TILE_RISK = 30;
const short MODE1860_PAIR_TILE_CAREER = 31;
const short MODE1860_PAIR_TILE_FAMILY = 32;
const short MODE1860_PAIR_TILE_BASIC = 33;
const short MODE1860_PAIR_TILE_MARRIAGE = 34;
const short MODE1860_PAIR_TILE_WORK = 35;
const short MODE1860_PAIR_TILE_COLLEGE = 36;
const short MODE1860_COLOR_MUSTARD = 34;
const short MODE1860_COLOR_TEAL_LIGHT = 35;
const short MODE1860_COLOR_TEAL_DARK = 36;
const short MODE1860_COLOR_BRICK = 37;
const short MODE1860_COLOR_MAUVE = 38;
const short MODE1860_COLOR_OLIVE = 39;
const short MODE1860_COLOR_PAYDAY = 40;
const short MODE1860_COLOR_DARK_WARM = 41;

//Input: areaLeft (left boundary), areaWidth (width of area), textWidth (width of text)->(all integer)
//Output: integer x-coordinate
//Purpose: calculates the centered x-position for text inside a given area
//Relation: used by drawTileBox and other rendering functions to align text properly
int centeredX(int areaLeft, int areaWidth, int textWidth) {
    return areaLeft + std::max(0, (areaWidth - textWidth) / 2);
}

//Input: text (string), maxWidth (integer maximum allowed width)
//Output: truncated or original string
//Purpose: ensures text fits within a given width
//Relation: used by tileCaption and drawClassicTile to prevent overflow
std::string clipText(const std::string& text, int maxWidth) {
    if (maxWidth <= 0) {
        return "";
    }
    if (static_cast<int>(text.size()) <= maxWidth) {
        return text;
    }
    return text.substr(0, static_cast<std::size_t>(maxWidth));
}

//Input: Tile reference
//Output: string title for the tile
//Purpose: maps tile kind and id ranges to human-readable names
//Relation: used in drawTileBox to display tile titles
std::string tileTitle(const Tile& tile) {
    switch (tile.kind) {
        case TILE_BLACK:
            if (tile.id >= 14 && tile.id <= 24) return "College";
            if (tile.id >= 26 && tile.id <= 36) return "Career";
            if (tile.id >= 59 && tile.id <= 68) return "Family";
            if (tile.id >= 69 && tile.id <= 78) return "Work";
            if (tile.id >= 80 && tile.id <= 82) return "Safe";
            if (tile.id >= 83 && tile.id <= 85) return "Risk";
            return "Action";
        case TILE_START: return "Start";
        case TILE_SPLIT_START: return "Choice";
        case TILE_COLLEGE: return "College";
        case TILE_CAREER: return "Career";
        case TILE_GRADUATION: return "Grad";
        case TILE_MARRIAGE: return "Married";
        case TILE_SPLIT_FAMILY: return "Family";
        case TILE_FAMILY: return "Family";
        case TILE_NIGHT_SCHOOL: return "Night";
        case TILE_SPLIT_RISK: return "Fork";
        case TILE_SAFE: return "Safe";
        case TILE_RISKY: return "Risk";
        case TILE_SPIN_AGAIN: return "Spin";
        case TILE_CAREER_2: return "Work";
        case TILE_PAYDAY: return "Payday";
        case TILE_BABY: return "Baby";
        case TILE_RETIREMENT: return "Retire";
        case TILE_HOUSE: return "House";
        case TILE_EMPTY:
        default:
            return "Road";
    }
}

//Input: Tile reference
//Output: string caption (short abbreviation)
//Purpose: generates a clipped caption for the tile
//Relation: complements tileTitle in drawTileBox
std::string tileCaption(const Tile& tile) {
    return clipText(getTileAbbreviation(tile), TILE_W - 2);
}

//Input: Tile reference
//Output: integer color pair index
//Purpose: determines which color scheme to use for rendering a tile
//Relation: used in drawTileBox and drawClassicTile for ncurses color attributes
int tileColorPair(const Tile& tile) {
    switch (tile.kind) {
        case TILE_BLACK:
        case TILE_RISKY:
        case TILE_MARRIAGE:
        case TILE_SPLIT_START:
        case TILE_SPLIT_FAMILY:
        case TILE_SPLIT_RISK:
        case TILE_HOUSE:
            return 5;
        case TILE_START:
        case TILE_RETIREMENT:
        case TILE_PAYDAY:
        case TILE_GRADUATION:
        case TILE_BABY:
        case TILE_NIGHT_SCHOOL:
        case TILE_SPIN_AGAIN:
            return 4;
        default:
            return 3;
    }
}

//Input: vector of Player objects, integer tileIndex
//Output: vector of player indices at that tile
//Purpose: finds which players occupy a given tile
//Relation: used in drawTileBox and drawClassicTokens to render player tokens
std::vector<int> playersAtTile(const std::vector<Player>& players, int tileIndex) {
    std::vector<int> indices;
    for (std::size_t i = 0; i < players.size(); ++i) {
        if (players[i].tile == tileIndex) {
            indices.push_back(static_cast<int>(i));
        }
    }
    return indices;
}

//Input: vector of Tile objects
//Output: vector of pairs (tile index, next tile index)
//Purpose: builds connections between tiles based on next and altNext
//Relation: used for pathfinding and board visualization
std::vector<std::pair<int, int> > boardConnections(const std::vector<Tile>& tiles) {
    std::vector<std::pair<int, int> > connections;
    for (std::size_t i = 0; i < tiles.size(); ++i) {
        if (tiles[i].next >= 0) {
            connections.push_back(std::make_pair(static_cast<int>(i), tiles[i].next));
        }
        if (tiles[i].altNext >= 0) {
            connections.push_back(std::make_pair(static_cast<int>(i), tiles[i].altNext));
        }
    }
    return connections;
}

//Input: vector of Tile objects
//Output: set of reachable tile indices
//Purpose: computes all tiles reachable from start using BFS-like traversal
//Relation: ensures board connectivity and validates tile graph
std::set<int> reachableTiles(const std::vector<Tile>& tiles) {
    std::set<int> reachable;
    std::vector<int> pending;
    pending.push_back(0);

    while (!pending.empty()) {
        const int current = pending.back();
        pending.pop_back();
        if (current < 0 || current >= static_cast<int>(tiles.size()) || reachable.count(current) != 0) {
            continue;
        }

        reachable.insert(current);
        if (tiles[static_cast<std::size_t>(current)].next >= 0) {
            pending.push_back(tiles[static_cast<std::size_t>(current)].next);
        }
        if (tiles[static_cast<std::size_t>(current)].altNext >= 0) {
            pending.push_back(tiles[static_cast<std::size_t>(current)].altNext);
        }
    }

    return reachable;
}

//Input: vector of Tile objects, Player reference, tileId
//Output: integer next tile index
//Purpose: determines next tile based on player?s choices (start, family, risk)
//Relation: used in buildVisibleTrail to navigate forward
int nextTileForView(const std::vector<Tile>& tiles, const Player& player, int tileId) {
    const Tile& tile = tiles[static_cast<std::size_t>(tileId)];
    if (tile.kind == TILE_SPLIT_START || tile.kind == TILE_START) {
        if (player.startChoice == 0) {
            return tile.next;
        }
        if (player.startChoice == 1) {
            return tile.altNext;
        }
        return tile.next;
    }
    if (tile.kind == TILE_SPLIT_FAMILY) {
        if (player.familyChoice == 0) {
            return tile.next;
        }
        if (player.familyChoice == 1) {
            return tile.altNext;
        }
        return tile.altNext;
    }
    if (tile.kind == TILE_SPLIT_RISK) {
        if (player.riskChoice == 0) {
            return tile.next;
        }
        if (player.riskChoice == 1) {
            return tile.altNext;
        }
        return tile.next;
    }
    return tile.next;
}

//Input: vector of Tile objects, Player reference, tileId
//Output: integer previous tile index
//Purpose: finds previous tile(s) leading to given tile, considering splits
//Relation: used in buildVisibleTrail to navigate backward
int previousTileForView(const std::vector<Tile>& tiles, const Player& player, int tileId) {
    std::vector<int> candidates;
    for (std::size_t i = 0; i < tiles.size(); ++i) {
        if (tiles[i].next == tileId || tiles[i].altNext == tileId) {
            candidates.push_back(static_cast<int>(i));
        }
    }
    if (candidates.empty()) {
        return -1;
    }
    if (candidates.size() == 1) {
        return candidates[0];
    }
    if (tileId == 38){ // College/Career split
        return player.startChoice == 0 ? 24 : 37;
    }
    if (tileId == 1) { //Path after graduation
        return player.startChoice == 0 ? 37 : 36;
    }
    if (tileId == 79) { // Family split
        return player.familyChoice == 0 ? 68 : 78;
    }
    if (tileId == 87) { // Risky/Safe split
        return 86;
    }
    if (tileId == 86) { // Retirement split
        return player.riskChoice == 0 ? 82 : 85;
    }
    return candidates[0];
}

//Input: vector of Tile objects, Player reference, focusTile
//Output: set of visible tile indices
//Purpose: builds a trail of tiles around focusTile for rendering camera view
//Relation: combines nextTileForView and previousTileForView to show context
std::set<int> buildVisibleTrail(const std::vector<Tile>& tiles, const Player& player, int focusTile) {
    std::set<int> visible;
    visible.insert(focusTile);

    int cursor = focusTile;
    for (int i = 0; i < 3; ++i) {
        cursor = previousTileForView(tiles, player, cursor);
        if (cursor < 0) {
            break;
        }
        visible.insert(cursor);
        if (tiles[static_cast<std::size_t>(cursor)].stop) {
            break;
        }
    }

    cursor = focusTile;
    for (int i = 0; i < 6; ++i) {
        cursor = nextTileForView(tiles, player, cursor);
        if (cursor < 0) {
            break;
        }
        visible.insert(cursor);
        if (tiles[static_cast<std::size_t>(cursor)].stop) {
            break;
        }
    }

    const Tile& focus = tiles[static_cast<std::size_t>(focusTile)];
    if (focus.kind == TILE_START || focus.kind == TILE_SPLIT_START) {
        if (focus.next >= 0) {
            visible.insert(focus.next);
        }
        if (focus.altNext >= 0) {
            visible.insert(focus.altNext);
        }
    }
    if (focus.kind == TILE_SPLIT_FAMILY || focus.kind == TILE_SPLIT_RISK) {
        if (focus.next >= 0) {
            visible.insert(focus.next);
        }
        if (focus.altNext >= 0) {
            visible.insert(focus.altNext);
        }
    }

    return visible;
}

//Input: WINDOW pointer, coordinates (y1,x1,y2,x2), hasColor, colorPair, maxY, maxX
//Output: none
//Purpose: draws a line segment on the board window
//Relation: used in drawClassicBoardGrid and tree guides
void drawLineSegment(WINDOW* win, int y1, int x1, int y2, int x2, bool hasColor, int colorPair, int maxY, int maxX) {
    if (hasColor) {
        wattron(win, COLOR_PAIR(colorPair) | A_DIM);
    } else {
        wattron(win, A_DIM);
    }

    int x = x1;
    int y = y1;
    while (x != x2) {
        x += (x2 > x) ? 1 : -1;
        if (y > 0 && y < maxY - 1 && x > 0 && x < maxX - 1) {
            drawBorderCharSafe(win, y, x, ACS_HLINE);
        }
    }
    while (y != y2) {
        y += (y2 > y) ? 1 : -1;
        if (y > 0 && y < maxY - 1 && x > 0 && x < maxX - 1) {
            drawBorderCharSafe(win, y, x, ACS_VLINE);
        }
    }

    if (hasColor) {
        wattroff(win, COLOR_PAIR(colorPair) | A_DIM);
    } else {
        wattroff(win, A_DIM);
    }
}

//Input: WINDOW pointer, coordinates (y,x), players vector, tokenIndices vector, hasColor
//Output: none
//Purpose: draws player tokens inside a tile box
//Relation: used in drawTileBox to show which players occupy a tile
void drawTokenString(WINDOW* boardWin,
                     int y,
                     int x,
                     const std::vector<Player>& players,
                     const std::vector<int>& tokenIndices,
                     bool hasColor) {
    const int maxTokens = tokenIndices.size() > 3 ? 2 : static_cast<int>(tokenIndices.size());
    std::string tokenText = "[";
    for (int i = 0; i < maxTokens; ++i) {
        tokenText.push_back(players[static_cast<std::size_t>(tokenIndices[static_cast<std::size_t>(i)])].token);
    }
    if (tokenIndices.size() > 3) {
        tokenText.push_back('+');
    } else if (tokenIndices.size() == 3) {
        tokenText.push_back(players[static_cast<std::size_t>(tokenIndices[2])].token);
    } else if (tokenIndices.empty()) {
        tokenText.push_back(' ');
    }
    tokenText.push_back(']');

    const int drawX = x + std::max(0, ((TILE_W - 2) - static_cast<int>(tokenText.size())) / 2);
    mvwprintw(boardWin, y, drawX, "%s", tokenText.c_str());

    for (std::size_t i = 0; i < tokenIndices.size(); ++i) {
        if (i >= 3) {
            break;
        }
        const int tokenOffset = drawX + 1 + static_cast<int>(i);
        if (hasColor) {
            wattron(boardWin, COLOR_PAIR(PLAYER_PAIR_BASE + (tokenIndices[i] % 4)) | A_BOLD);
        } else {
            wattron(boardWin, A_BOLD);
        }
        mvwaddch(boardWin, y, tokenOffset, players[static_cast<std::size_t>(tokenIndices[i])].token);
        if (hasColor) {
            wattroff(boardWin, COLOR_PAIR(PLAYER_PAIR_BASE + (tokenIndices[i] % 4)) | A_BOLD);
        } else {
            wattroff(boardWin, A_BOLD);
        }
    }
}

//Input: WINDOW pointer, Tile reference, players vector, tileLeft, tileTop, isFocusTile, hasColor
//Output: none
//Purpose: draws a full tile box with title, caption, and tokens
//Relation: central rendering function for board tiles
void drawTileBox(WINDOW* boardWin,
                 const Tile& tile,
                 const std::vector<Player>& players,
                 int tileLeft,
                 int tileTop,
                 bool isFocusTile,
                 bool hasColor) {
    const int borderPair = isFocusTile ? 8 : 2;
    const int textPair = isFocusTile ? 8 : (tile.kind == TILE_EMPTY ? 6 : tileColorPair(tile));
    const int attrs = isFocusTile ? A_BOLD : A_DIM;

    if (hasColor) {
        wattron(boardWin, COLOR_PAIR(borderPair) | attrs);
    } else {
        wattron(boardWin, attrs);
    }

    drawBoxAtSafe(boardWin, tileTop, tileLeft, TILE_H, TILE_W);

    if (hasColor) {
        wattroff(boardWin, COLOR_PAIR(borderPair) | attrs);
    } else {
        wattroff(boardWin, attrs);
    }

    const std::string title = clipText(tileTitle(tile), TILE_W - 2);
    const std::string caption = tileCaption(tile);
    const std::vector<int> tilePlayers = playersAtTile(players, tile.id);

    if (hasColor) {
        wattron(boardWin, COLOR_PAIR(textPair) | attrs);
    } else {
        wattron(boardWin, attrs);
    }
    mvwprintw(boardWin, tileTop + 1, centeredX(tileLeft + 1, TILE_W - 2, static_cast<int>(title.size())), "%s", title.c_str());
    if (hasColor) {
        wattroff(boardWin, COLOR_PAIR(textPair) | attrs);
    } else {
        wattroff(boardWin, attrs);
    }

    drawTokenString(boardWin, tileTop + 3, tileLeft + 1, players, tilePlayers, hasColor);

    if (hasColor) {
        wattron(boardWin, COLOR_PAIR(textPair) | attrs);
    } else {
        wattron(boardWin, attrs);
    }
    mvwprintw(boardWin, tileTop + TILE_H - 2,
              centeredX(tileLeft + 1, TILE_W - 2, static_cast<int>(caption.size())),
              "%s",
              caption.c_str());
    if (hasColor) {
        wattroff(boardWin, COLOR_PAIR(textPair) | attrs);
    } else {
        wattroff(boardWin, attrs);
    }
}

//Input: WINDOW pointer, hasColor flag
//Output: none
//Purpose: draw static board decorations (grid, trees, region labels, landmarks)
//Relation: used in Board::render to build the full board background
void drawClassicBoardGrid(WINDOW* boardWin, bool hasColor) {
    if (hasColor) {
        wattron(boardWin, COLOR_PAIR(GOLDRUSH_BROWN_SAND) | A_DIM);
    } else {
        wattron(boardWin, A_DIM);
    }
    for (std::size_t i = 0; i < sizeof(CLASSIC_ROW_SEGMENTS) / sizeof(CLASSIC_ROW_SEGMENTS[0]); ++i) {
        const RowSegment& row = CLASSIC_ROW_SEGMENTS[i];
        const int left = CLASSIC_BOARD_POSITIONS[row.startIndex].x - 1;
        const int width = row.count * 4 + 1;
        drawBorderLineSafe(boardWin, row.y - 1, left, width);
        drawBorderLineSafe(boardWin, row.y + 1, left, width);
        for (int col = 0; col <= row.count; ++col) {
            drawBorderCharSafe(boardWin, row.y, left + (col * 4), ACS_VLINE);
        }
        drawBorderCharSafe(boardWin, row.y - 1, left, ACS_ULCORNER);
        drawBorderCharSafe(boardWin, row.y + 1, left, ACS_LLCORNER);
        for (int col = 1; col < row.count; ++col) {
            drawBorderCharSafe(boardWin, row.y - 1, left + (col * 4), ACS_TTEE);
            drawBorderCharSafe(boardWin, row.y + 1, left + (col * 4), ACS_BTEE);
        }
        drawBorderCharSafe(boardWin, row.y - 1, left + width - 1, ACS_URCORNER);
        drawBorderCharSafe(boardWin, row.y + 1, left + width - 1, ACS_LRCORNER);
    }
    if (hasColor) {
        wattroff(boardWin, COLOR_PAIR(GOLDRUSH_BROWN_SAND) | A_DIM);
    } else {
        wattroff(boardWin, A_DIM);
    }
}

//Input: WINDOW pointer, hasColor flag
//Output: none
//Purpose: draw static board decorations (grid, trees, region labels, landmarks)
//Relation: used in Board::render to build the full board background
void drawClassicTreeGuides(WINDOW* boardWin, bool hasColor) {
    if (hasColor) {
        wattron(boardWin, COLOR_PAIR(GOLDRUSH_BROWN_SAND) | A_BOLD);
    } else {
        wattron(boardWin, A_BOLD);
    }
    drawBorderColumnSafe(boardWin, 3, 40, 1);
    drawBorderColumnSafe(boardWin, 5, 16, 2);
    drawBorderLineSafe(boardWin, 6, 17, 22);
    drawBorderColumnSafe(boardWin, 5, 40, 2);
    drawBorderColumnSafe(boardWin, 8, 40, 1);
    drawBorderColumnSafe(boardWin, 11, 40, 1);
    drawBorderColumnSafe(boardWin, 14, 40, 1);
    drawBorderLineSafe(boardWin, 15, 40, 2);
    drawBorderColumnSafe(boardWin, 15, 22, 2);
    drawBorderLineSafe(boardWin, 18, 23, 17);
    drawBorderColumnSafe(boardWin, 18, 58, 4);
    drawBorderColumnSafe(boardWin, 23, 40, 1);
    if (hasColor) {
        wattroff(boardWin, COLOR_PAIR(GOLDRUSH_BROWN_SAND) | A_BOLD);
    } else {
        wattroff(boardWin, A_BOLD);
    }
}

//Input: WINDOW pointer, hasColor flag
//Output: none
//Purpose: draw static board decorations (grid, trees, region labels, landmarks)
//Relation: used in Board::render to build the full board background
void drawClassicRegions(WINDOW* boardWin, bool hasColor) {
    for (std::size_t i = 0; i < sizeof(CLASSIC_REGION_LABELS) / sizeof(CLASSIC_REGION_LABELS[0]); ++i) {
        if (hasColor) {
            wattron(boardWin, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);
        } else {
            wattron(boardWin, A_BOLD);
        }
        mvwprintw(boardWin, CLASSIC_REGION_LABELS[i].y, CLASSIC_REGION_LABELS[i].x,
                  "%s", CLASSIC_REGION_LABELS[i].name);
        if (hasColor) {
            wattroff(boardWin, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);
        } else {
            wattroff(boardWin, A_BOLD);
        }
    }
}

//Input: WINDOW pointer, hasColor flag
//Output: none
//Purpose: draw static board decorations (grid, trees, region labels, landmarks)
//Relation: used in Board::render to build the full board background
void drawClassicLandmarks(WINDOW* boardWin, bool hasColor) {
    if (hasColor) {
        wattron(boardWin, COLOR_PAIR(GOLDRUSH_BLACK_CREAM) | A_BOLD);
    } else {
        wattron(boardWin, A_BOLD);
    }
    mvwprintw(boardWin, 2, 3, "BANK");
    mvwprintw(boardWin, 3, 2, "/----\\");
    mvwprintw(boardWin, 4, 2, "| $$ |");

    mvwprintw(boardWin, 8, 67, "UNI");
    mvwprintw(boardWin, 9, 66, "/---\\");
    mvwprintw(boardWin, 10, 66, "| U |");

    mvwprintw(boardWin, 14, 63, "[777]");
    mvwprintw(boardWin, 15, 61, "LUCK HALL");

    mvwprintw(boardWin, 26, 3, "RETIRE");
    mvwprintw(boardWin, 27, 4, "/\\/\\");
    if (hasColor) {
        wattroff(boardWin, COLOR_PAIR(GOLDRUSH_BLACK_CREAM) | A_BOLD);
    } else {
        wattroff(boardWin, A_BOLD);
    }
}

//Input: WINDOW pointer, Tile reference, hasColor flag
//Output: none
//Purpose: draws a tile symbol at its board position
//Relation: used in Board::render to show each tile
void drawClassicTile(WINDOW* boardWin, const Tile& tile, bool hasColor) {
    const UiPos pos = CLASSIC_BOARD_POSITIONS[tile.id];
    const int y = pos.y;
    const int x = pos.x - 1;
    const std::string label = getTileBoardSymbol(tile);
    const int colorPair = getTileColorPair(tile);
    const bool importantTile = tile.stop ||
                               tile.kind == TILE_PAYDAY ||
                               tile.kind == TILE_HOUSE ||
                               tile.kind == TILE_RETIREMENT ||
                               tile.kind == TILE_RISKY ||
                               tile.kind == TILE_CAREER ||
                               tile.kind == TILE_COLLEGE;

    if (hasColor) {
        wattron(boardWin, COLOR_PAIR(colorPair) | (importantTile ? A_BOLD : A_NORMAL));
        if (tile.kind == TILE_EMPTY) {
            wattron(boardWin, A_DIM);
        }
    } else if (importantTile) {
        wattron(boardWin, A_BOLD);
    }

    mvwaddch(boardWin, y, x, '[');
    mvwprintw(boardWin, y, x + 1, "%-2s", clipText(label, 2).c_str());
    mvwaddch(boardWin, y, x + 3, ']');

    if (hasColor) {
        wattroff(boardWin, COLOR_PAIR(colorPair) | (importantTile ? A_BOLD : A_NORMAL));
        if (tile.kind == TILE_EMPTY) {
            wattroff(boardWin, A_DIM);
        }
    } else if (importantTile) {
        wattroff(boardWin, A_BOLD);
    }
}

//Input: WINDOW pointer, players vector, tileIndex, currentPlayerIndex, highlightedTile, hasColor
//Output: none
//Purpose: draws player tokens on the classic board view
//Relation: complements drawClassicTile by overlaying player positions
void drawClassicTokens(WINDOW* boardWin,
                       const std::vector<Player>& players,
                       int tileIndex,
                       int currentPlayerIndex,
                       int highlightedTile,
                       bool hasColor) {
    std::vector<int> occupants = playersAtTile(players, tileIndex);
    if (occupants.empty()) {
        return;
    }

    const bool highlighted = highlightedTile == tileIndex;
    int colorPlayer = occupants[0];
    for (std::size_t i = 0; i < occupants.size(); ++i) {
        if (occupants[i] == currentPlayerIndex) {
            colorPlayer = currentPlayerIndex;
            break;
        }
    }

    std::string marker;
    if (highlighted && currentPlayerIndex >= 0 && currentPlayerIndex < static_cast<int>(players.size())) {
        marker = "<P" + std::to_string(currentPlayerIndex + 1);
        if (marker.size() < 4) {
            marker += ">";
        }
        marker = clipText(marker, 4);
    } else if (occupants.size() == 1) {
        marker = "[";
        marker.push_back(players[static_cast<std::size_t>(occupants[0])].token);
        marker += " ]";
    } else {
        marker = "[" + std::to_string(occupants.size()) + "P]";
    }
    if (marker.size() < 4) {
        marker += std::string(4 - marker.size(), ' ');
    }

    const UiPos pos = CLASSIC_BOARD_POSITIONS[tileIndex];
    const int y = pos.y;
    const int x = pos.x - 1;
    if (hasColor) {
        wattron(boardWin, COLOR_PAIR(PLAYER_PAIR_BASE + (colorPlayer % 4)) | A_BOLD | A_REVERSE);
    } else {
        wattron(boardWin, A_BOLD | A_REVERSE);
    }
    mvwprintw(boardWin, y, x, "%-.4s", marker.c_str());
    if (hasColor) {
        wattroff(boardWin, COLOR_PAIR(PLAYER_PAIR_BASE + (colorPlayer % 4)) | A_BOLD | A_REVERSE);
    } else {
        wattroff(boardWin, A_BOLD | A_REVERSE);
    }
}

//Input: Tile reference
//Output: 1860-flavored tile name
//Purpose: gives 1860 mode historical-style names without changing tile effects
//Relation: used by renderMode1860TileDetails
std::string mode1860FlavorName(const Tile& tile) {
    switch (tile.kind) {
        case TILE_START:
            return "Infancy / Start";
        case TILE_CAREER:
            return "Industry / Job";
        case TILE_CAREER_2:
            return "Industry / Work";
        case TILE_PAYDAY:
            return "Prosperity / Payday";
        case TILE_BLACK:
            return tile.value >= 3 ? "Contest / Minigame" : "Fate / Action";
        case TILE_RISKY:
            return "Misfortune / Risk";
        case TILE_SAFE:
            return "Virtue / Safe";
        case TILE_MARRIAGE:
            return "Matrimony / Marriage";
        case TILE_FAMILY:
        case TILE_BABY:
            return "Family";
        case TILE_COLLEGE:
            return "Schooling / College";
        case TILE_GRADUATION:
            return "Graduation";
        case TILE_NIGHT_SCHOOL:
            return "Night School";
        case TILE_SPLIT_START:
            return "Choice / College-Career";
        case TILE_SPLIT_FAMILY:
            return "Choice / Family-Life";
        case TILE_SPLIT_RISK:
            return "Choice / Safe-Risk";
        case TILE_SPIN_AGAIN:
            return "Second Spin";
        case TILE_HOUSE:
            return "Homestead / House";
        case TILE_RETIREMENT:
            return "Happy Old Age / Retirement";
        case TILE_EMPTY:
        default:
            return "Open Road";
    }
}

//Input: Tile reference
//Output: short 1860 display abbreviation
//Purpose: keeps tile details compact while preserving full names elsewhere
//Relation: used by renderMode1860TileDetails
std::string mode1860Abbreviation(const Tile& tile) {
    switch (tile.kind) {
        case TILE_START:
            return "IN";
        case TILE_CAREER:
            return "JB";
        case TILE_CAREER_2:
            return "PR";
        case TILE_PAYDAY:
            return "SP";
        case TILE_BLACK:
            return tile.value >= 3 ? "MG" : "FT";
        case TILE_RISKY:
            return "MS";
        case TILE_SAFE:
            return "VT";
        case TILE_MARRIAGE:
            return "MT";
        case TILE_FAMILY:
            return "FM";
        case TILE_BABY:
            return "FE";
        case TILE_COLLEGE:
            return "SC";
        case TILE_GRADUATION:
            return "GR";
        case TILE_SPLIT_START:
            return "CH";
        case TILE_SPLIT_FAMILY:
            return "FL";
        case TILE_SPLIT_RISK:
            return "SR";
        case TILE_RETIREMENT:
            return tile.label == "CA" ? "CA" : "HO";
        case TILE_HOUSE:
            return "HS";
        default:
            return getTileAbbreviation(tile);
    }
}

//Input: viewport top coordinate and visible 1860 tile
//Output: screen y coordinate for the tile cell
//Purpose: converts 1860 grid rows to terminal cell positions
//Relation: used by 1860 tile and token rendering
int mode1860CellTop(int top, const Tile& tile) {
    return top + (tile.mode1860Y * MODE1860_CELL_H);
}

//Input: viewport left coordinate and visible 1860 tile
//Output: screen x coordinate for the tile cell
//Purpose: converts 1860 grid columns to terminal cell positions
//Relation: used by 1860 tile and token rendering
int mode1860CellLeft(int left, const Tile& tile) {
    return left + (tile.mode1860X * MODE1860_CELL_W);
}

//Input: Tile reference
//Output: true when the tile has a valid 1860 grid position
//Purpose: rejects invalid or non-1860 tile coordinates during rendering
//Relation: used by 1860 board, token, and detail rendering
bool tileHasMode1860Position(const Tile& tile) {
    return tile.mode1860Y >= 0 &&
           tile.mode1860Y < MODE1860_ROWS &&
           tile.mode1860X >= 0 &&
           tile.mode1860X < MODE1860_COLS;
}

//Input: hasColor flag from the UI
//Output: true when 1860-specific color pairs are available
//Purpose: initializes solid 1860 tile and checkerboard colors safely
//Relation: used by drawMode1860Board before drawing cells
bool initMode1860ColorPairs(bool hasColor) {
    if (!hasColor || COLOR_PAIRS <= MODE1860_PAIR_TILE_COLLEGE) {
        return false;
    }

    short mustard = COLOR_YELLOW;
    short darkWarm = COLOR_BLACK;
    short tealLight = COLOR_CYAN;
    short tealDark = COLOR_BLUE;
    short brick = COLOR_RED;
    short mauve = COLOR_MAGENTA;
    short olive = COLOR_GREEN;
    short payday = COLOR_GREEN;

    if (can_change_color() && COLORS > MODE1860_COLOR_DARK_WARM) {
        init_color(MODE1860_COLOR_MUSTARD, 980, 792, 471);
        init_color(MODE1860_COLOR_TEAL_LIGHT, 408, 780, 757);
        init_color(MODE1860_COLOR_TEAL_DARK, 102, 224, 251);
        init_color(MODE1860_COLOR_BRICK, 867, 325, 255);
        init_color(MODE1860_COLOR_MAUVE, 706, 510, 608);
        init_color(MODE1860_COLOR_OLIVE, 333, 322, 149);
        init_color(MODE1860_COLOR_PAYDAY, 329, 506, 275);
        init_color(MODE1860_COLOR_DARK_WARM, 80, 60, 40);
        mustard = MODE1860_COLOR_MUSTARD;
        darkWarm = MODE1860_COLOR_DARK_WARM;
        tealLight = MODE1860_COLOR_TEAL_LIGHT;
        tealDark = MODE1860_COLOR_TEAL_DARK;
        brick = MODE1860_COLOR_BRICK;
        mauve = MODE1860_COLOR_MAUVE;
        olive = MODE1860_COLOR_OLIVE;
        payday = MODE1860_COLOR_PAYDAY;
    }

    init_pair(MODE1860_PAIR_CHECKER_LIGHT, COLOR_BLACK, mustard);
    init_pair(MODE1860_PAIR_CHECKER_DARK, COLOR_WHITE, darkWarm);
    init_pair(MODE1860_PAIR_TILE_ROUTE, COLOR_BLACK, mustard);
    init_pair(MODE1860_PAIR_TILE_PAYDAY, COLOR_BLACK, payday);
    init_pair(MODE1860_PAIR_TILE_ACTION, COLOR_BLACK, tealLight);
    init_pair(MODE1860_PAIR_TILE_MINIGAME, COLOR_BLACK, mauve);
    init_pair(MODE1860_PAIR_TILE_RISK, COLOR_WHITE, brick);
    init_pair(MODE1860_PAIR_TILE_CAREER, COLOR_WHITE, tealDark);
    init_pair(MODE1860_PAIR_TILE_FAMILY, COLOR_BLACK, mauve);
    init_pair(MODE1860_PAIR_TILE_BASIC, COLOR_BLACK, mustard);
    init_pair(MODE1860_PAIR_TILE_MARRIAGE, COLOR_WHITE, brick);
    init_pair(MODE1860_PAIR_TILE_WORK, COLOR_BLACK, olive);
    init_pair(MODE1860_PAIR_TILE_COLLEGE, COLOR_BLACK, tealLight);
    return true;
}

//Input: 1860 board row and column
//Output: checkerboard background color pair
//Purpose: alternates base colors behind playable 1860 cells
//Relation: used by drawMode1860Board
int mode1860CheckerColorPair(int row, int col) {
    return ((row + col) % 2) == 0 ? MODE1860_PAIR_CHECKER_LIGHT
                                  : MODE1860_PAIR_CHECKER_DARK;
}

//Input: Tile reference
//Output: 1860 tile color pair
//Purpose: maps each gameplay tile type to a consistent solid color
//Relation: used by drawMode1860Tile
int mode1860TileColorPair(const Tile& tile) {
    switch (tile.kind) {
        case TILE_PAYDAY:
        case TILE_SAFE:
            return MODE1860_PAIR_TILE_PAYDAY;
        case TILE_BLACK:
            return tile.value >= 3 ? MODE1860_PAIR_TILE_MINIGAME : MODE1860_PAIR_TILE_ACTION;
        case TILE_RISKY:
        case TILE_SPLIT_RISK:
            return MODE1860_PAIR_TILE_RISK;
        case TILE_COLLEGE:
        case TILE_GRADUATION:
        case TILE_NIGHT_SCHOOL:
            return MODE1860_PAIR_TILE_COLLEGE;
        case TILE_CAREER:
            return MODE1860_PAIR_TILE_CAREER;
        case TILE_CAREER_2:
            return MODE1860_PAIR_TILE_WORK;
        case TILE_MARRIAGE:
            return MODE1860_PAIR_TILE_MARRIAGE;
        case TILE_SPLIT_FAMILY:
        case TILE_FAMILY:
        case TILE_BABY:
        case TILE_HOUSE:
            return MODE1860_PAIR_TILE_FAMILY;
        case TILE_START:
        case TILE_SPLIT_START:
        case TILE_SPIN_AGAIN:
        case TILE_RETIREMENT:
            return MODE1860_PAIR_TILE_ROUTE;
        case TILE_EMPTY:
        default:
            return MODE1860_PAIR_TILE_BASIC;
    }
}

//Input: Tile reference
//Output: fallback character for terminals without color
//Purpose: preserves readable 1860 categories when color pairs are unavailable
//Relation: used by drawMode1860Tile
char mode1860FallbackFill(const Tile& tile) {
    switch (tile.kind) {
        case TILE_PAYDAY:
        case TILE_SAFE:
            return '+';
        case TILE_BLACK:
            return tile.value >= 3 ? '*' : '?';
        case TILE_RISKY:
        case TILE_SPLIT_RISK:
            return '!';
        case TILE_CAREER:
        case TILE_CAREER_2:
        case TILE_COLLEGE:
        case TILE_GRADUATION:
        case TILE_NIGHT_SCHOOL:
            return '=';
        case TILE_MARRIAGE:
        case TILE_SPLIT_FAMILY:
        case TILE_FAMILY:
        case TILE_BABY:
        case TILE_HOUSE:
            return '@';
        case TILE_START:
        case TILE_SPLIT_START:
        case TILE_SPIN_AGAIN:
        case TILE_RETIREMENT:
            return '#';
        case TILE_EMPTY:
        default:
            return ' ';
    }
}

//Input: Tile reference
//Output: tiny symbol shown inside important 1860 cells
//Purpose: avoids dense cell text while still marking major spaces
//Relation: used by drawMode1860Tile
std::string mode1860TileSymbol(const Tile& tile) {
    switch (tile.kind) {
        case TILE_START:
            return "S";
        case TILE_RETIREMENT:
            return "R";
        case TILE_BLACK:
            return tile.value >= 3 ? "M" : "A";
        case TILE_RISKY:
        case TILE_SPLIT_RISK:
            return "!";
        case TILE_SAFE:
            return "+";
        case TILE_CAREER:
        case TILE_CAREER_2:
        case TILE_COLLEGE:
        case TILE_GRADUATION:
        case TILE_NIGHT_SCHOOL:
            return "J";
        case TILE_MARRIAGE:
        case TILE_SPLIT_FAMILY:
        case TILE_FAMILY:
        case TILE_BABY:
        case TILE_HOUSE:
            return "F";
        case TILE_PAYDAY:
        case TILE_SPLIT_START:
        case TILE_SPIN_AGAIN:
        case TILE_EMPTY:
        default:
            return "";
    }
}

//Input: Tile reference
//Output: concise text describing the tile effect
//Purpose: moves full 1860 tile meaning into the detail panel
//Relation: used by renderMode1860TileDetails
std::string mode1860EffectHint(const Tile& tile) {
    switch (tile.kind) {
        case TILE_START:
            return "Choose college or career path.";
        case TILE_COLLEGE:
            return "Begin the college path.";
        case TILE_CAREER:
            return "Choose or draw a career.";
        case TILE_CAREER_2:
            return "Work and career event.";
        case TILE_GRADUATION:
            return "Graduate and enter the main route.";
        case TILE_PAYDAY:
            return "Receive salary.";
        case TILE_BLACK:
            return tile.value >= 3 ? "Play a minigame." : "Resolve an action or life event.";
        case TILE_RISKY:
            return "Resolve a risk event.";
        case TILE_SAFE:
            return "Take the safer route reward.";
        case TILE_MARRIAGE:
            return "Resolve marriage.";
        case TILE_SPLIT_FAMILY:
            return "Choose family or life route.";
        case TILE_FAMILY:
        case TILE_BABY:
            return "Resolve a family event.";
        case TILE_NIGHT_SCHOOL:
            return "Improve career options.";
        case TILE_SPLIT_RISK:
            return "Choose safe or risky route.";
        case TILE_SPIN_AGAIN:
            return "Spin again.";
        case TILE_HOUSE:
            return "Resolve a house event.";
        case TILE_RETIREMENT:
            return "Retire and prepare final scoring.";
        case TILE_SPLIT_START:
            return "Choose a starting route.";
        case TILE_EMPTY:
        default:
            return "Advance along the path.";
    }
}

//Input: board window, rectangle coordinates/dimensions, and fill character
//Output: none
//Purpose: paints a solid block for one 1860 checker or tile cell
//Relation: used by drawMode1860Board and drawMode1860Tile
void fillMode1860Rect(WINDOW* boardWin,
                      int y,
                      int x,
                      int height,
                      int width,
                      chtype fill) {
    const int maxY = getmaxy(boardWin);
    const int maxX = getmaxx(boardWin);
    for (int dy = 0; dy < height; ++dy) {
        for (int dx = 0; dx < width; ++dx) {
            const int drawY = y + dy;
            const int drawX = x + dx;
            if (drawY > 0 && drawY < maxY - 1 && drawX > 0 && drawX < maxX - 1) {
                mvwaddch(boardWin, drawY, drawX, fill);
            }
        }
    }
}

//Input: board window, tile, viewport origin, highlight flags, and color flag
//Output: none
//Purpose: draws one solid 1860 playable tile over the checkerboard background
//Relation: used by drawMode1860Board
void drawMode1860Tile(WINDOW* boardWin,
                      const Tile& tile,
                      int top,
                      int left,
                      bool highlighted,
                      bool reachable,
                      bool hasColor) {
    if (!tileHasMode1860Position(tile)) {
        return;
    }

    const int cellTop = mode1860CellTop(top, tile);
    const int cellLeft = mode1860CellLeft(left, tile);
    const int colorPair = mode1860TileColorPair(tile);
    const int fillAttrs = highlighted ? (A_BOLD | A_REVERSE) : (reachable ? A_BOLD : A_NORMAL);
    const chtype fill = hasColor
        ? (' ' | COLOR_PAIR(colorPair) | fillAttrs)
        : (mode1860FallbackFill(tile) |
           (highlighted ? (A_BOLD | A_REVERSE) : (reachable ? A_BOLD : A_DIM)));

    fillMode1860Rect(boardWin, cellTop, cellLeft, MODE1860_CELL_H, MODE1860_CELL_W, fill);

    const std::string symbol = mode1860TileSymbol(tile);
    if (symbol.empty()) {
        return;
    }

    const int labelY = cellTop + (MODE1860_CELL_H / 2);
    const int labelX = cellLeft + std::max(0, (MODE1860_CELL_W - static_cast<int>(symbol.size())) / 2);
    const int labelAttrs = A_BOLD | (highlighted ? A_REVERSE : (reachable ? A_UNDERLINE : A_NORMAL));
    if (hasColor) {
        wattron(boardWin, COLOR_PAIR(colorPair) | labelAttrs);
    } else {
        wattron(boardWin, labelAttrs);
    }
    mvwprintw(boardWin, labelY, labelX, "%s", symbol.c_str());
    if (hasColor) {
        wattroff(boardWin, COLOR_PAIR(colorPair) | labelAttrs);
    } else {
        wattroff(boardWin, labelAttrs);
    }
}

//Input: player indices occupying one tile
//Output: compact player marker text
//Purpose: keeps one or more players visible on the same 1860 cell
//Relation: used by drawMode1860Tokens
std::string mode1860PlayerMarker(const std::vector<int>& occupants) {
    if (occupants.empty()) {
        return "";
    }

    if (occupants.size() == 1U) {
        return "P" + std::to_string(occupants.front() + 1);
    }

    std::string marker = "P";
    for (std::size_t i = 0; i < occupants.size(); ++i) {
        marker += std::to_string(occupants[i] + 1);
    }
    return clipText(marker, MODE1860_CELL_W - 1);
}

//Input: board window, marker position/text, player color, focus flag, and color flag
//Output: none
//Purpose: draws a highly visible 1860 player marker above tile colors
//Relation: used by drawMode1860Tokens
void drawMode1860PlayerMarker(WINDOW* boardWin,
                              int y,
                              int x,
                              const std::string& marker,
                              int colorPlayer,
                              bool containsFocusPlayer,
                              bool hasColor) {
    const int attrs = A_BOLD | A_REVERSE | (containsFocusPlayer ? A_UNDERLINE : A_NORMAL);
    if (hasColor) {
        wattron(boardWin, COLOR_PAIR(PLAYER_PAIR_BASE + (colorPlayer % 4)) | attrs);
    } else {
        wattron(boardWin, attrs);
    }
    mvwprintw(boardWin, y, x, "%s", marker.c_str());
    if (hasColor) {
        wattroff(boardWin, COLOR_PAIR(PLAYER_PAIR_BASE + (colorPlayer % 4)) | attrs);
    } else {
        wattroff(boardWin, attrs);
    }
}

//Input: board window, players, visible tiles, viewport origin, focus player, and color flag
//Output: none
//Purpose: overlays player markers on the 1860 camera board
//Relation: used by drawMode1860Board after tiles are painted
void drawMode1860Tokens(WINDOW* boardWin,
                        const std::vector<Player>& players,
                        const std::vector<Tile>& tiles,
                        int top,
                        int left,
                        int focusPlayerIndex,
                        bool hasColor) {
    for (std::size_t tileIndex = 0; tileIndex < tiles.size(); ++tileIndex) {
        const Tile& tile = tiles[tileIndex];
        if (!tileHasMode1860Position(tile)) {
            continue;
        }

        const std::vector<int> occupants = playersAtTile(players, tile.id);
        if (occupants.empty()) {
            continue;
        }

        std::string marker = mode1860PlayerMarker(occupants);
        int colorPlayer = occupants.front();
        bool containsFocusPlayer = occupants.front() == focusPlayerIndex;
        for (std::size_t i = 0; i < occupants.size(); ++i) {
            if (occupants[i] == focusPlayerIndex) {
                colorPlayer = occupants[i];
                containsFocusPlayer = true;
                break;
            }
        }

        const int y = mode1860CellTop(top, tile) + (MODE1860_CELL_H / 2);
        const int x = mode1860CellLeft(left, tile) +
                      std::max(0, (MODE1860_CELL_W - static_cast<int>(marker.size())) / 2);
        drawMode1860PlayerMarker(boardWin, y, x, marker, colorPlayer, containsFocusPlayer, hasColor);
    }
}

//Input: board window, row/column, label, color/fallback, color flag, and max width
//Output: next x coordinate after the legend item
//Purpose: draws one compact legend swatch for 1860 tile categories
//Relation: used by renderMode1860Legend
int drawMode1860LegendItem(WINDOW* boardWin,
                           int y,
                           int x,
                           const std::string& label,
                           int colorPair,
                           char fallback,
                           bool hasColor,
                           int maxX) {
    if (x >= maxX - 2) {
        return x;
    }

    if (hasColor) {
        wattron(boardWin, COLOR_PAIR(colorPair) | A_BOLD);
        mvwprintw(boardWin, y, x, "  ");
        wattroff(boardWin, COLOR_PAIR(colorPair) | A_BOLD);
        x += 3;
    } else {
        const std::string swatch = std::string("[") + fallback + "]";
        mvwprintw(boardWin, y, x, "%s", clipText(swatch, maxX - x - 1).c_str());
        x += static_cast<int>(swatch.size()) + 1;
    }

    const std::string clipped = clipText(label, maxX - x - 1);
    if (!clipped.empty()) {
        mvwprintw(boardWin, y, x, "%s", clipped.c_str());
    }
    return x + static_cast<int>(clipped.size()) + 2;
}

//Input: board window, row, and color flag
//Output: none
//Purpose: draws the 1860 color legend
//Relation: used by drawMode1860Board
void renderMode1860Legend(WINDOW* boardWin, int y, bool hasColor) {
    const int maxX = getmaxx(boardWin);
    int x = 2;

    if (hasColor) {
        wattron(boardWin, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);
    } else {
        wattron(boardWin, A_BOLD);
    }
    mvwprintw(boardWin, y, x, "Legend:");
    if (hasColor) {
        wattroff(boardWin, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);
    } else {
        wattroff(boardWin, A_BOLD);
    }
    x += 9;

    x = drawMode1860LegendItem(boardWin, y, x, "Pay/Safe", MODE1860_PAIR_TILE_PAYDAY, '+', hasColor, maxX);
    x = drawMode1860LegendItem(boardWin, y, x, "Action", MODE1860_PAIR_TILE_ACTION, '?', hasColor, maxX);
    x = drawMode1860LegendItem(boardWin, y, x, "Mini", MODE1860_PAIR_TILE_MINIGAME, '*', hasColor, maxX);
    x = drawMode1860LegendItem(boardWin, y, x, "Risk", MODE1860_PAIR_TILE_RISK, '!', hasColor, maxX);
    x = drawMode1860LegendItem(boardWin, y, x, "Job", MODE1860_PAIR_TILE_CAREER, '=', hasColor, maxX);
    drawMode1860LegendItem(boardWin, y, x, "Family", MODE1860_PAIR_TILE_FAMILY, '@', hasColor, maxX);
}

//Input: board window, row, and color flag
//Output: none
//Purpose: explains the tiny symbols used on important 1860 spaces
//Relation: used by drawMode1860Board when there is enough terminal height
void renderMode1860SymbolLegend(WINDOW* boardWin, int y, bool hasColor) {
    const int maxX = getmaxx(boardWin);
    const std::string symbols = "Symbols: S Start  R Retire  A Action  M Mini  ! Risk  + Safe  J Job  F Family";
    const std::string clipped = clipText(symbols, maxX - 4);
    if (hasColor) {
        wattron(boardWin, COLOR_PAIR(GOLDRUSH_BROWN_CREAM));
    } else {
        wattron(boardWin, A_DIM);
    }
    mvwprintw(boardWin,
              y,
              centeredX(1, maxX - 2, static_cast<int>(clipped.size())),
              "%s",
              clipped.c_str());
    if (hasColor) {
        wattroff(boardWin, COLOR_PAIR(GOLDRUSH_BROWN_CREAM));
    } else {
        wattroff(boardWin, A_DIM);
    }
}

//Input: players and tile id
//Output: human-readable occupant list
//Purpose: shows which players are on the selected 1860 tile
//Relation: used by renderMode1860TileDetails
std::string mode1860PlayersAtTileText(const std::vector<Player>& players, int tileIndex) {
    std::string text = "Players here:";
    bool found = false;
    for (std::size_t i = 0; i < players.size(); ++i) {
        if (players[i].tile != tileIndex) {
            continue;
        }
        text += found ? ", " : " ";
        text += "P" + std::to_string(static_cast<int>(i) + 1) + " " + players[i].name;
        found = true;
    }
    return found ? text : "Players here: none";
}

//Input: board window, 1860 tile list, players, focus tile, row, and color flag
//Output: none
//Purpose: renders full selected/current 1860 tile details outside the board cells
//Relation: used by drawMode1860Board
void renderMode1860TileDetails(WINDOW* boardWin,
                               const std::vector<Tile>& tiles,
                               const std::vector<Player>& players,
                               int focusTile,
                               int y,
                               bool hasColor) {
    const int maxY = getmaxy(boardWin);
    const int maxX = getmaxx(boardWin);
    const int localTile = focusTile - MODE1860_BASE_TILE_ID;
    if (y <= 0 ||
        y >= maxY - 1 ||
        localTile < 0 ||
        localTile >= static_cast<int>(tiles.size())) {
        return;
    }

    const Tile& tile = tiles[static_cast<std::size_t>(localTile)];
    const std::string current =
        "Current Tile: " + getTileFullName(tile) + " (" + mode1860Abbreviation(tile) + ") - " +
        mode1860EffectHint(tile);
    const std::string occupants = mode1860PlayersAtTileText(players, focusTile);
    const std::string flavor =
        "1860 Style: " + mode1860FlavorName(tile) + " | Space " + std::to_string(focusTile) +
        " | " + occupants;

    if (hasColor) {
        wattron(boardWin, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);
    } else {
        wattron(boardWin, A_BOLD);
    }
    mvwprintw(boardWin, y, 2, "%s", clipText(current, maxX - 4).c_str());
    if (hasColor) {
        wattroff(boardWin, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);
    } else {
        wattroff(boardWin, A_BOLD);
    }

    if (y + 1 < maxY - 1) {
        mvwprintw(boardWin, y + 1, 2, "%s", clipText(flavor, maxX - 4).c_str());
    }
}

//Input: board window, 1860 tiles, players, focus/cursor ids, reachable set, remaining movement, and color flag
//Output: none
//Purpose: draws the 1860 camera-follow board and manual movement highlights
//Relation: used by Board::render and Board::render1860Selection
void drawMode1860Board(WINDOW* boardWin,
                       const std::vector<Tile>& tiles,
                       const std::vector<Player>& players,
                       int focusIndex,
                       int focusTile,
                       int cursorTile,
                       const std::set<int>& reachableTiles,
                       int movementSteps,
                       bool hasColor) {
    const int maxY = getmaxy(boardWin);
    const int maxX = getmaxx(boardWin);
    const bool useModeColors = initMode1860ColorPairs(hasColor);
    const int visibleRows = maxY >= 31 ? 11 : 9;
    const int visibleCols = maxX >= 82 ? 11 : 9;
    const int fallbackTile = MODE1860_BASE_TILE_ID + ((MODE1860_ROWS - 1) * MODE1860_COLS);
    if (focusTile < MODE1860_BASE_TILE_ID ||
        focusTile >= MODE1860_BASE_TILE_ID + static_cast<int>(tiles.size())) {
        focusTile = fallbackTile;
    }
    if (cursorTile >= 0 &&
        (cursorTile < MODE1860_BASE_TILE_ID ||
         cursorTile >= MODE1860_BASE_TILE_ID + static_cast<int>(tiles.size()))) {
        cursorTile = -1;
    }
    const Tile& cameraTile = cursorTile >= 0 ? tiles[static_cast<std::size_t>(cursorTile - MODE1860_BASE_TILE_ID)]
                                             : tiles[static_cast<std::size_t>(focusTile - MODE1860_BASE_TILE_ID)];
    const int cameraRow = std::max(0, std::min(cameraTile.mode1860Y - (visibleRows / 2), MODE1860_ROWS - visibleRows));
    const int cameraCol = std::max(0, std::min(cameraTile.mode1860X - (visibleCols / 2), MODE1860_COLS - visibleCols));
    const int boardW = visibleCols * MODE1860_CELL_W;
    const int boardH = visibleRows * MODE1860_CELL_H;
    const int borderW = boardW + 2;
    const bool roomy = maxY >= 31;
    const int startY = roomy ? 5 : 4;
    const int startX = std::max(2, ((maxX - borderW) / 2) + 1);

    const std::string title = movementSteps > 0
        ? "1860 Manual Movement"
        : "1860 Free Movement Board";
    mvwprintw(boardWin,
              1,
              centeredX(1, maxX - 2, static_cast<int>(title.size())),
              "%s",
              clipText(title, maxX - 4).c_str());
    renderMode1860Legend(boardWin, 2, useModeColors);
    if (roomy) {
        renderMode1860SymbolLegend(boardWin, 3, useModeColors);
    }
    for (int row = 0; row < visibleRows; ++row) {
        for (int col = 0; col < visibleCols; ++col) {
            const int boardRow = cameraRow + row;
            const int boardCol = cameraCol + col;
            const bool light = ((boardRow + boardCol) % 2) == 0;
            const chtype fill = useModeColors
                ? (' ' | COLOR_PAIR(mode1860CheckerColorPair(boardRow, boardCol)))
                : ((light ? '.' : ' ') | A_DIM);
            fillMode1860Rect(boardWin,
                             startY + (row * MODE1860_CELL_H),
                             startX + (col * MODE1860_CELL_W),
                             MODE1860_CELL_H,
                             MODE1860_CELL_W,
                             fill);
        }
    }

    if (useModeColors) {
        wattron(boardWin, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);
    } else {
        wattron(boardWin, A_BOLD);
    }
    drawBoxAtSafe(boardWin, startY - 1, startX - 1, boardH + 2, boardW + 2);
    if (useModeColors) {
        wattroff(boardWin, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);
    } else {
        wattroff(boardWin, A_BOLD);
    }

    for (std::size_t i = 0; i < tiles.size(); ++i) {
        const Tile& tile = tiles[i];
        if (!tileHasMode1860Position(tile) ||
            tile.mode1860Y < cameraRow ||
            tile.mode1860Y >= cameraRow + visibleRows ||
            tile.mode1860X < cameraCol ||
            tile.mode1860X >= cameraCol + visibleCols) {
            continue;
        }
        Tile drawTile = tile;
        drawTile.mode1860Y -= cameraRow;
        drawTile.mode1860X -= cameraCol;
        drawMode1860Tile(boardWin,
                         drawTile,
                         startY,
                         startX,
                         tile.id == focusTile || tile.id == cursorTile,
                         reachableTiles.count(tile.id) != 0,
                         useModeColors);
    }
    std::vector<Tile> visibleTiles = tiles;
    for (std::size_t i = 0; i < visibleTiles.size(); ++i) {
        if (visibleTiles[i].mode1860Y < cameraRow ||
            visibleTiles[i].mode1860Y >= cameraRow + visibleRows ||
            visibleTiles[i].mode1860X < cameraCol ||
            visibleTiles[i].mode1860X >= cameraCol + visibleCols) {
            visibleTiles[i].mode1860Y = -1;
            visibleTiles[i].mode1860X = -1;
        } else {
            visibleTiles[i].mode1860Y -= cameraRow;
            visibleTiles[i].mode1860X -= cameraCol;
        }
    }
    drawMode1860Tokens(boardWin, players, visibleTiles, startY, startX, focusIndex, useModeColors);

    const int detailsY = startY + boardH + 1;
    if (detailsY < maxY - 1) {
        const int detailTile = cursorTile >= 0 ? cursorTile : focusTile;
        renderMode1860TileDetails(boardWin, tiles, players, detailTile, detailsY, useModeColors);
    }

    if (movementSteps > 0 && maxY > 4) {
        const std::string instruction =
            "Remaining: " + std::to_string(movementSteps) +
            " | Move one highlighted adjacent tile. Enter stops. Esc/Q cancels before moving or stops after moving.";
        mvwprintw(boardWin, maxY - 2, 2, "%s", clipText(instruction, maxX - 4).c_str());
    }
}

}  // namespace

//Input: BoardViewMode enum or string name
//Output: string name or BoardViewMode enum
//Purpose: convert between enum and string representations
//Relation: used in UI to switch board view modes
std::string boardViewModeName(BoardViewMode mode) {
    switch (mode) {
        case BoardViewMode::ClassicFull:
            return "classic-full";
        case BoardViewMode::Mode1860:
            return "1860";
        case BoardViewMode::FollowCamera:
        default:
            return "follow-camera";
    }
}

//Input: BoardViewMode enum or string name
//Output: string name or BoardViewMode enum
//Purpose: convert between enum and string representations
//Relation: used in UI to switch board view modes
BoardViewMode boardViewModeFromName(const std::string& name) {
    std::string normalized;
    normalized.reserve(name.size());
    for (std::size_t i = 0; i < name.size(); ++i) {
        const char ch = static_cast<char>(std::tolower(static_cast<unsigned char>(name[i])));
        if (ch != ' ' && ch != '_' && ch != '-' && ch != '/') {
            normalized.push_back(ch);
        }
    }

    if (normalized == "classic" ||
        normalized == "classicfull" ||
        normalized == "classicfullboard" ||
        normalized == "fullboard") {
        return BoardViewMode::ClassicFull;
    }
    if (normalized == "1860" ||
        normalized == "mode1860" ||
        normalized == "1860mode" ||
        normalized == "checkered1860" ||
        normalized == "1860checkered" ||
        normalized == "1860checkeredboard" ||
        normalized == "checkeredboardmode" ||
        normalized == "checkeredlifemode") {
        return BoardViewMode::Mode1860;
    }
    return BoardViewMode::FollowCamera;
}

//Input: none
//Output: none
//Purpose: initializes tiles and regions
//Relation: sets up the board state for gameplay
Board::Board() {
    initTiles();
    initRegions();
}

//Input: integer id
//Output: const Tile reference
//Purpose: retrieves tile by id
//Relation: used throughout rendering and logic functions
const Tile& Board::tileAt(int id) const {
    if (id >= MODE1860_BASE_TILE_ID) {
        const int index = id - MODE1860_BASE_TILE_ID;
        if (index >= 0 && index < static_cast<int>(mode1860Tiles.size())) {
            return mode1860Tiles[static_cast<std::size_t>(index)];
        }
    }
    return tiles[std::max(0, std::min(id, static_cast<int>(tiles.size()) - 1))];
}

//Input: none
//Output: total count of classic and 1860 tile ids
//Purpose: exposes the full valid tile id range
//Relation: used by traps, saves, and debug helpers
int Board::tileCount() const {
    return MODE1860_BASE_TILE_ID + static_cast<int>(mode1860Tiles.size());
}

//Input: integer tile id
//Output: true when the id belongs to the 1860 board
//Purpose: distinguishes expanded 1860 ids from classic tile ids
//Relation: used by movement, rendering, traps, and save/load repair
bool Board::isMode1860TileId(int id) const {
    return id >= MODE1860_BASE_TILE_ID &&
           id < MODE1860_BASE_TILE_ID + static_cast<int>(mode1860Tiles.size());
}

//Input: integer tile id
//Output: true if the 1860 tile can be occupied
//Purpose: filters invalid or blank 1860 coordinates before movement, traps, or saves use them
//Relation: used by 1860 movement and save/load repair
bool Board::isMode1860WalkableTile(int id) const {
    if (!isMode1860TileId(id)) {
        return false;
    }
    const Tile& tile = tileAt(id);
    return tile.mode1860Y >= 0 &&
           tile.mode1860Y < MODE1860_ROWS &&
           tile.mode1860X >= 0 &&
           tile.mode1860X < MODE1860_COLS;
}

//Input: none
//Output: 1860 start tile id
//Purpose: centralizes the 1860 start coordinate
//Relation: used by setup, movement repair, and save/load migration
int Board::mode1860StartTileId() const {
    return mode1860TileIdAt(MODE1860_ROWS - 1, 0);
}

//Input: none
//Output: 1860 retirement tile id
//Purpose: centralizes the 1860 retirement coordinate
//Relation: used by movement, rendering, retirement, and save/load migration
int Board::mode1860RetirementTileId() const {
    return mode1860TileIdAt(0, MODE1860_COLS - 1);
}

//Input: none
//Output: 1860 board row count
//Purpose: exposes 1860 dimensions without duplicating constants
//Relation: used by game movement, rendering, and save/load
int Board::mode1860Rows() const {
    return MODE1860_ROWS;
}

//Input: none
//Output: 1860 board column count
//Purpose: exposes 1860 dimensions without duplicating constants
//Relation: used by game movement, rendering, and save/load
int Board::mode1860Cols() const {
    return MODE1860_COLS;
}

//Input: 1860 row and column
//Output: tile id or -1 if coordinates are out of bounds
//Purpose: converts grid coordinates into actual 1860 tile ids
//Relation: used by manual movement and camera rendering
int Board::mode1860TileIdAt(int row, int col) const {
    if (row < 0 || row >= MODE1860_ROWS || col < 0 || col >= MODE1860_COLS) {
        return -1;
    }
    return MODE1860_BASE_TILE_ID + (row * MODE1860_COLS) + col;
}

//Input: 1860 row and column
//Output: life progression zone from 0 to 5
//Purpose: approximates life stage by distance from Start
//Relation: used by tile generation and region names
int Board::mode1860LifeZone(int row, int col) const {
    const int fromStart = std::max(0, (MODE1860_ROWS - 1 - row) + col);
    const int maxDistance = (MODE1860_ROWS - 1) + (MODE1860_COLS - 1);
    return std::min(5, (fromStart * 6) / (maxDistance + 1));
}

//Input: center tile id and requested visible rows/columns
//Output: clamped 1860 camera viewport rectangle
//Purpose: keeps the focused player or cursor visible on the large 1860 board
//Relation: used by 1860 camera-follow rendering
BoardRect Board::mode1860CameraViewport(int centerTileId, int visibleRows, int visibleCols) const {
    const Tile& center = tileAt(centerTileId);
    const int rows = std::max(5, std::min(MODE1860_ROWS, visibleRows));
    const int cols = std::max(5, std::min(MODE1860_COLS, visibleCols));
    BoardRect rect;
    rect.rows = rows;
    rect.cols = cols;
    rect.row = std::max(0, std::min(center.mode1860Y - (rows / 2), MODE1860_ROWS - rows));
    rect.col = std::max(0, std::min(center.mode1860X - (cols / 2), MODE1860_COLS - cols));
    return rect;
}

//Input: start tile id and exact movement step count
//Output: 1860 tile ids at exact Manhattan distance
//Purpose: supports legacy/debug 1860 reachability checks
//Relation: retained for debug and compatibility with earlier 1860 movement helpers
std::vector<int> Board::reachable1860Tiles(int startTileId, int steps) const {
    std::vector<int> reachable;
    if (!isMode1860WalkableTile(startTileId) || steps < 0) {
        return reachable;
    }
    const Tile& start = tileAt(startTileId);
    for (int row = 0; row < MODE1860_ROWS; ++row) {
        for (int col = 0; col < MODE1860_COLS; ++col) {
            const int tileId = mode1860TileIdAt(row, col);
            if (isMode1860WalkableTile(tileId) &&
                std::abs(row - start.mode1860Y) + std::abs(col - start.mode1860X) == steps) {
                reachable.push_back(tileId);
            }
        }
    }
    return reachable;
}

//Input: from tile id, destination tile id, and exact movement step count
//Output: true when the Manhattan distance matches the requested steps
//Purpose: supports legacy/debug exact-distance validation
//Relation: retained for debug and compatibility with earlier 1860 movement helpers
bool Board::isValid1860Move(int fromTileId, int toTileId, int steps) const {
    if (!isMode1860WalkableTile(fromTileId) || !isMode1860WalkableTile(toTileId) || steps < 0) {
        return false;
    }
    const Tile& from = tileAt(fromTileId);
    const Tile& to = tileAt(toTileId);
    return std::abs(from.mode1860Y - to.mode1860Y) + std::abs(from.mode1860X - to.mode1860X) == steps;
}

//Input: none
//Output: none
//Purpose: initializes all tiles with positions, labels, kinds, connections
//Relation: core setup for board structure
void Board::initTiles() {
    tiles.resize(TILE_COUNT);
    for (int i = 0; i < TILE_COUNT; ++i) {
        tiles[i].id = i;
        tiles[i].y = 0;
        tiles[i].x = 0;
        tiles[i].mode1860Y = -1;
        tiles[i].mode1860X = -1;
        tiles[i].label = "  ";
        tiles[i].kind = TILE_EMPTY;
        tiles[i].next = (i < TILE_COUNT - 1) ? i + 1 : -1;
        tiles[i].altNext = -1;
        tiles[i].value = 0;
        tiles[i].stop = false;
    }

    const auto place = [&](int id, int col, int row) {
        tiles[id].x = col;
        tiles[id].y = row;
    };
    const auto place1860 = [&](int id, int row, int col) {
        tiles[id].mode1860Y = row;
        tiles[id].mode1860X = col;
    };

    place(0, 1, 0);

    place(1, 9, 5);
    place(2, 8, 5);
    place(3, 7, 5);
    place(4, 6, 5);
    place(5, 5, 5);
    place(6, 5, 4);
    place(7, 5, 3);
    place(8, 5, 2);
    place(9, 6, 2);
    place(10, 7, 2);
    place(11, 8, 2);
    place(12, 9, 2);

    place(13, 0, 1);
    place(14, 0, 2);
    place(15, 0, 3);
    place(16, 0, 4);
    place(17, 1, 4);
    place(18, 2, 4);
    place(19, 3, 4);
    place(20, 4, 4);
    place(21, 5, 4);
    place(22, 6, 4);
    place(23, 7, 4);
    place(24, 8, 4);

    place(25, 2, 1);
    place(26, 3, 1);
    place(27, 4, 1);
    place(28, 5, 1);
    place(29, 6, 1);
    place(30, 7, 1);
    place(31, 8, 1);
    place(32, 9, 1);
    place(33, 10, 1);
    place(34, 10, 2);
    place(35, 10, 3);
    place(36, 10, 4);
    place(37, 9, 4);

    place(38, 4, 8);
    place(39, 3, 6);
    place(40, 4, 6);
    place(41, 6, 9);
    place(42, 7, 9);
    place(43, 8, 9);
    place(44, 11, 4);
    place(45, 11, 5);
    place(46, 11, 6);
    place(47, 11, 7);
    place(48, 11, 8);
    place(49, 11, 9);
    place(50, 11, 10);
    place(51, 11, 11);
    place(52, 11, 12);
    place(53, 10, 12);
    place(54, 9, 12);
    place(55, 8, 12);
    place(56, 7, 12);
    place(57, 6, 12);
    place(58, 5, 12);

    place(59, 10, 3);
    place(60, 9, 3);
    place(61, 8, 3);
    place(62, 7, 3);
    place(63, 7, 4);
    place(64, 7, 5);
    place(65, 7, 6);
    place(66, 7, 7);
    place(67, 7, 8);
    place(68, 7, 9);

    place(69, 10, 2);
    place(70, 9, 2);
    place(71, 8, 2);
    place(72, 7, 2);
    place(73, 6, 2);
    place(74, 5, 2);
    place(75, 5, 3);
    place(76, 5, 4);
    place(77, 5, 5);
    place(78, 5, 6);

    place(79, 6, 9);
    place(80, 0, 10);
    place(81, 0, 11);
    place(82, 0, 12);
    place(83, 1, 11);
    place(84, 1, 12);
    place(85, 1, 13);
    place(86, 3, 10);
    place(87, 4, 10);
    place(88, 4, 10);

    place1860(0, 10, 0);
    place1860(1, 6, 6);
    place1860(2, 6, 5);
    place1860(3, 6, 4);
    place1860(4, 6, 3);
    place1860(5, 5, 3);
    place1860(6, 5, 4);
    place1860(7, 5, 5);
    place1860(8, 5, 6);
    place1860(9, 5, 7);
    place1860(10, 5, 8);
    place1860(11, 5, 9);
    place1860(12, 5, 10);

    place1860(13, 9, 0);
    place1860(14, 8, 0);
    place1860(15, 7, 0);
    place1860(16, 7, 1);
    place1860(17, 7, 2);
    place1860(18, 8, 2);
    place1860(19, 9, 2);
    place1860(20, 9, 3);
    place1860(21, 9, 4);
    place1860(22, 8, 4);
    place1860(23, 7, 4);
    place1860(24, 7, 5);

    place1860(25, 10, 1);
    place1860(26, 10, 2);
    place1860(27, 10, 3);
    place1860(28, 10, 4);
    place1860(29, 10, 5);
    place1860(30, 10, 6);
    place1860(31, 10, 7);
    place1860(32, 10, 8);
    place1860(33, 9, 8);
    place1860(34, 8, 8);
    place1860(35, 7, 8);
    place1860(36, 7, 7);
    place1860(37, 7, 6);

    place1860(38, 6, 0);
    place1860(39, 6, 1);
    place1860(40, 6, 2);
    place1860(41, 5, 0);
    place1860(42, 5, 1);
    place1860(43, 5, 2);
    place1860(44, 4, 0);
    place1860(45, 4, 1);
    place1860(46, 4, 2);
    place1860(47, 4, 3);
    place1860(48, 4, 4);
    place1860(49, 4, 5);
    place1860(50, 4, 6);
    place1860(51, 4, 7);
    place1860(52, 3, 0);
    place1860(53, 3, 1);
    place1860(54, 2, 0);
    place1860(55, 2, 1);
    place1860(56, 1, 0);
    place1860(57, 1, 1);
    place1860(58, 0, 0);

    place1860(59, 4, 10);
    place1860(60, 3, 10);
    place1860(61, 2, 10);
    place1860(62, 2, 9);
    place1860(63, 2, 8);
    place1860(64, 2, 7);
    place1860(65, 2, 6);
    place1860(66, 2, 5);
    place1860(67, 2, 4);
    place1860(68, 2, 3);

    place1860(69, 4, 9);
    place1860(70, 4, 8);
    place1860(71, 3, 8);
    place1860(72, 3, 7);
    place1860(73, 3, 6);
    place1860(74, 3, 5);
    place1860(75, 3, 4);
    place1860(76, 3, 3);
    place1860(77, 3, 2);
    place1860(78, 2, 2);

    place1860(79, 1, 2);
    place1860(80, 1, 3);
    place1860(81, 1, 4);
    place1860(82, 1, 5);
    place1860(83, 0, 2);
    place1860(84, 0, 3);
    place1860(85, 0, 4);
    place1860(86, 0, 6);
    place1860(87, 0, 9);
    place1860(88, 0, 10);

    for (int i = 1; i <= 12; ++i) {
        tiles[i].label = "BK";
        tiles[i].kind = TILE_BLACK;
        tiles[i].value = 2;
    }

    tiles[0].label = "ST";
    tiles[0].kind = TILE_START;
    tiles[0].next = 13;
    tiles[0].altNext = 25;

    tiles[13].label = "O";
    tiles[13].kind = TILE_COLLEGE;
    tiles[13].stop = true;
    for (int i = 14; i <= 24; ++i) {
        tiles[i].label = "BK";
        tiles[i].kind = TILE_BLACK;
        tiles[i].value = 1;
    }
    tiles[22].label = "PD";
    tiles[22].kind = TILE_PAYDAY;
    tiles[22].value = 5000;
    tiles[24].next = 37;

    tiles[25].label = "A";
    tiles[25].kind = TILE_CAREER;
    tiles[25].stop = true;
    for (int i = 26; i <= 37; ++i) {
        tiles[i].label = "BK";
        tiles[i].kind = TILE_BLACK;
        tiles[i].value = 1;
    }
    tiles[31].label = "PD";
    tiles[31].kind = TILE_PAYDAY;
    tiles[31].value = 7000;
    tiles[37].label = "GR";
    tiles[37].kind = TILE_GRADUATION;
    tiles[37].stop = true;
    tiles[37].next = 1;
    tiles[36].next = 1;

    tiles[38].label = "BK";
    tiles[38].kind = TILE_BLACK;
    tiles[38].value = 2;
    tiles[38].stop = false;
    tiles[38].next = 39;

    for (int i = 39; i <= 58; ++i) {
        tiles[i].label = "BK";
        tiles[i].kind = TILE_BLACK;
        tiles[i].value = 2;
    }
    tiles[12].next = 58;
    tiles[41].label = "PD";
    tiles[41].kind = TILE_PAYDAY;
    tiles[41].value = 12000;
    tiles[9].label = "M";
    tiles[9].kind = TILE_MARRIAGE;
    tiles[9].stop = true;
    tiles[47].label = "PD";
    tiles[47].kind = TILE_PAYDAY;
    tiles[47].value = 15000;
    tiles[55].label = "PD";
    tiles[55].kind = TILE_PAYDAY;
    tiles[55].value = 18000;
    tiles[12].label = "FW";
    tiles[12].kind = TILE_SPLIT_FAMILY;
    tiles[12].stop = true;
    tiles[12].next = 59;
    tiles[12].altNext = 69;

    for (int i = 59; i <= 68; ++i) {
        tiles[i].label = "BK";
        tiles[i].kind = TILE_BLACK;
        tiles[i].value = 2;
    }
    tiles[59].label = "F";
    tiles[59].kind = TILE_FAMILY;
    tiles[59].stop = true;
    tiles[64].label = "PD";
    tiles[64].kind = TILE_PAYDAY;
    tiles[64].value = 20000;
    tiles[68].next = 79;

    for (int i = 69; i <= 78; ++i) {
        tiles[i].label = "BK";
        tiles[i].kind = TILE_BLACK;
        tiles[i].value = 2;
    }
    tiles[69].label = "W";
    tiles[69].kind = TILE_CAREER_2;
    tiles[69].value = 10000;
    tiles[75].label = "PD";
    tiles[75].kind = TILE_PAYDAY;
    tiles[75].value = 22000;
    tiles[78].next = 79;

    tiles[79].label = "SR";
    tiles[79].kind = TILE_SPLIT_RISK;
    tiles[79].stop = true;
    tiles[79].next = 80;
    tiles[79].altNext = 83;

    tiles[80].label = "S";
    tiles[80].kind = TILE_SAFE;
    tiles[80].stop = true;
    tiles[81].label = "BK";
    tiles[81].kind = TILE_BLACK;
    tiles[81].value = 2;
    tiles[82].label = "BK";
    tiles[82].kind = TILE_BLACK;
    tiles[82].value = 2;
    tiles[82].next = 86;

    tiles[83].label = "R";
    tiles[83].kind = TILE_RISKY;
    tiles[83].stop = true;
    tiles[84].label = "BK";
    tiles[84].kind = TILE_BLACK;
    tiles[84].value = 3;
    tiles[85].label = "BK";
    tiles[85].kind = TILE_BLACK;
    tiles[85].value = 3;
    tiles[85].next = 86;

    tiles[86].label = "BK";
    tiles[86].kind = TILE_BLACK;
    tiles[86].value = 3;
    tiles[86].next = 87;

    tiles[87].label = "RET";
    tiles[87].kind = TILE_RETIREMENT;
    tiles[87].stop = true;
    tiles[87].next = -1;

    tiles[88].label = "RET";
    tiles[88].kind = TILE_RETIREMENT;
    tiles[88].stop = true;
    tiles[88].next = -1;

    init1860FreeMovementBoard();
}

//Input: none
//Output: none
//Purpose: initializes every 1860 grid tile and places major life milestones
//Relation: called by initTiles and read by 1860 rendering/movement
void Board::init1860FreeMovementBoard() {
    mode1860Tiles.clear();
    mode1860Tiles.resize(MODE1860_ROWS * MODE1860_COLS);

    for (int row = 0; row < MODE1860_ROWS; ++row) {
        for (int col = 0; col < MODE1860_COLS; ++col) {
            const int localId = (row * MODE1860_COLS) + col;
            Tile& tile = mode1860Tiles[static_cast<std::size_t>(localId)];
            tile.id = MODE1860_BASE_TILE_ID + localId;
            tile.y = row;
            tile.x = col;
            tile.mode1860Y = row;
            tile.mode1860X = col;
            tile.next = -1;
            tile.altNext = -1;
            tile.value = 0;
            tile.stop = false;

            const int zone = mode1860LifeZone(row, col);
            const int pattern = ((row * 17) + (col * 31) + (zone * 7)) % 20;
            tile.label = "LF";
            tile.kind = TILE_EMPTY;

            if (pattern < 5) {
                tile.kind = TILE_BLACK;
                tile.label = "AC";
                tile.value = zone >= 3 ? 2 : 1;
            } else if (pattern < 8) {
                tile.kind = TILE_PAYDAY;
                tile.label = "PD";
                tile.value = 3000 + (zone * 4000);
            } else if (pattern < 10) {
                tile.kind = TILE_SAFE;
                tile.label = "SF";
                tile.stop = true;
            } else if (pattern < 12) {
                tile.kind = TILE_RISKY;
                tile.label = "RK";
                tile.stop = true;
            } else if (pattern < 14) {
                tile.kind = TILE_CAREER_2;
                tile.label = "PR";
                tile.value = 4000 + (zone * 2500);
            } else if (pattern < 16) {
                tile.kind = TILE_BLACK;
                tile.label = "MG";
                tile.value = 3;
            } else if (pattern == 16 && zone >= 3) {
                tile.kind = TILE_HOUSE;
                tile.label = "HS";
                tile.stop = true;
            } else if (pattern == 17 && zone >= 4) {
                tile.kind = TILE_BABY;
                tile.label = "FE";
                tile.stop = true;
            } else if (pattern == 18 && zone >= 2) {
                tile.kind = TILE_NIGHT_SCHOOL;
                tile.label = "NS";
                tile.stop = true;
            }

            if (zone == 0 && row == MODE1860_ROWS - 1 && col == 0) {
                tile.kind = TILE_START;
                tile.label = "ST";
                tile.stop = false;
            }
            if (zone == 1 && row == 18 && col == 6) {
                tile.kind = TILE_COLLEGE;
                tile.label = "CO";
                tile.stop = true;
            }
            if (zone == 1 && row == 19 && col == 8) {
                tile.kind = TILE_CAREER;
                tile.label = "JB";
                tile.stop = true;
            }
            if (zone == 2 && ((row == 15 && col == 8) ||
                              (row == 14 && col == 11))) {
                tile.kind = TILE_GRADUATION;
                tile.label = "GR";
                tile.stop = true;
            }
            if (zone == 3 && row == 10 && col == 11) {
                tile.kind = TILE_MARRIAGE;
                tile.label = "MR";
                tile.stop = true;
            }
            if (zone >= 4 && ((row == 8 && col == 17) || (row == 7 && col == 18))) {
                tile.kind = TILE_FAMILY;
                tile.label = "FM";
                tile.stop = true;
            }
            if (zone >= 5 && row == 0 && col == MODE1860_COLS - 1) {
                tile.kind = TILE_RETIREMENT;
                tile.label = "RET";
                tile.stop = true;
            }
            if (zone >= 5 && row == 1 && col == MODE1860_COLS - 2) {
                tile.kind = TILE_RETIREMENT;
                tile.label = "RET";
                tile.stop = true;
            }
        }
    }
}

//Input: Tile reference
//Output: bool
//Purpose: checks if tile is a stop space
//Relation: used in movement and rendering logic
bool Board::isStopSpace(const Tile& tile) const {
    return tile.stop;
}

//Input: tileIndex
//Output: string region name
//Purpose: finds which region a tile belongs to
//Relation: used in UI and tutorial legend
std::string Board::regionNameForTile(int tileIndex) const {
    if (isMode1860TileId(tileIndex)) {
        const Tile& tile = tileAt(tileIndex);
        switch (mode1860LifeZone(tile.mode1860Y, tile.mode1860X)) {
            case 0: return "Infancy";
            case 1: return "Early Life";
            case 2: return "College / Career";
            case 3: return "Career Growth";
            case 4: return "Family & Property";
            case 5: return "Late Life";
            default: break;
        }
    }
    for (std::size_t i = 0; i < regions.size(); ++i) {
        if (tileIndex >= regions[i].startTileIndex && tileIndex <= regions[i].endTileIndex) {
            return regions[i].name;
        }
    }
    return "Open Road";
}

//Input: none
//Output: vector of strings
//Purpose: builds tutorial legend with symbols and names
//Relation: used in help/tutorial UI
std::vector<std::string> Board::tutorialLegend() const {
    std::vector<std::string> lines;
    lines.push_back("[" + getTileBoardSymbol(tileAt(10)) + "] " + getTileDisplayName(tileAt(10)));
    lines.push_back("[" + getTileBoardSymbol(tileAt(12)) + "] " + getTileDisplayName(tileAt(12)));
    lines.push_back("[" + getTileBoardSymbol(tileAt(13)) + "] " + getTileDisplayName(tileAt(13)));
    lines.push_back("[" + getTileBoardSymbol(tileAt(25)) + "] " + getTileDisplayName(tileAt(25)));
    lines.push_back("[" + getTileBoardSymbol(tileAt(20)) + "] " + getTileDisplayName(tileAt(20)));
    lines.push_back("[" + getTileBoardSymbol(tileAt(39)) + "] " + getTileDisplayName(tileAt(39)));
    lines.push_back("[" + getTileBoardSymbol(tileAt(84)) + "] " + getTileDisplayName(tileAt(84)));
    lines.push_back("[" + getTileBoardSymbol(tileAt(68)) + "] " + getTileDisplayName(tileAt(68)));
    lines.push_back("[" + getTileBoardSymbol(tileAt(83)) + "] " + getTileDisplayName(tileAt(83)));
    lines.push_back("[" + getTileBoardSymbol(tileAt(0)) + "] " + getTileDisplayName(tileAt(0)));
    lines.push_back("[2P] Multiple players on one tile");
    return lines;
}

//Input: none
//Output: none
//Purpose: initializes board regions with names and tile ranges
//Relation: complements initTiles, used in regionNameForTile
void Board::initRegions() {
    regions.clear();
    regions.push_back({"Startup Street", 0, 12});
    regions.push_back({"Career City", 13, 38});
    regions.push_back({"Goldrush Valley", 39, 58});
    regions.push_back({"Family Avenue", 59, 72});
    regions.push_back({"Risky Road", 73, 86});
    regions.push_back({"Retirement Ridge", 87, 88});
}

// Input: boardWin (WINDOW* for ncurses output), players (vector containing all players),
//        focusPlayerIndex (index of the player that the camera focuses on),
//        highlightedTile (the tile currently being highlighted),
//        hasColor (flag indicating whether colors are enabled),
//        viewMode (board display mode: ClassicFull or FollowCamera)
// Output: none
// Purpose: draws the entire game board to the window,
//          including the grid, regions, landmarks, tiles, and player tokens.
// Relation: this is the main function for board visualization.
//           It calls drawClassicBoardGrid, drawClassicTreeGuides,
//           drawClassicRegions, drawClassicLandmarks, drawClassicTile,
//           drawClassicTokens, and drawTileBox to display details.
//           It depends on the result from buildVisibleTrail to determine
//          which tiles and connections to show based on the focused player's position.
void Board::render(WINDOW* boardWin,
                   const std::vector<Player>& players,
                   int focusPlayerIndex,
                   int highlightedTile,
                   bool hasColor,
                   BoardViewMode viewMode) const {
    werase(boardWin);
    drawBoxSafe(boardWin);

    if (players.empty()) {
        mvwprintw(boardWin, 1, 2, "Board view unavailable.");
        wrefresh(boardWin);
        return;
    }

    const int focusIndex = std::max(0, std::min(focusPlayerIndex, static_cast<int>(players.size()) - 1));
    const int centerTile = players[static_cast<std::size_t>(focusIndex)].tile;
    const int focusTile = highlightedTile >= 0 ? highlightedTile : centerTile;

    if (viewMode == BoardViewMode::ClassicFull) {
        const int maxY = getmaxy(boardWin);
        const int maxX = getmaxx(boardWin);
        const std::string statusLine =
            players[static_cast<std::size_t>(focusIndex)].name + " at Space " +
            std::to_string(centerTile) + " - " + getTileDisplayName(tileAt(centerTile));

        drawClassicBoardGrid(boardWin, hasColor);
        drawClassicTreeGuides(boardWin, hasColor);
        drawClassicRegions(boardWin, hasColor);
        drawClassicLandmarks(boardWin, hasColor);
        for (int i = 0; i < TILE_COUNT; ++i) {
            drawClassicTile(boardWin, tiles[static_cast<std::size_t>(i)], hasColor);
        }
        for (int i = 0; i < TILE_COUNT; ++i) {
            drawClassicTokens(boardWin, players, i, focusIndex, focusTile, hasColor);
        }
        if (maxY >= 31) {
            const int statusY = std::min(maxY - 2, 28);
            mvwprintw(boardWin,
                      statusY,
                      centeredX(1, maxX - 2, static_cast<int>(clipText(statusLine, maxX - 4).size())),
                      "%s",
                      clipText(statusLine, maxX - 4).c_str());
        }
        drawBoxSafe(boardWin);
        wrefresh(boardWin);
        return;
    }

    if (viewMode == BoardViewMode::Mode1860) {
        const std::set<int> noReachable;
        drawMode1860Board(boardWin,
                          mode1860Tiles,
                          players,
                          focusIndex,
                          focusTile,
                          -1,
                          noReachable,
                          0,
                          hasColor);
        drawBoxSafe(boardWin);
        wrefresh(boardWin);
        return;
    }

    const Tile& center = tileAt(centerTile);
    const int maxY = getmaxy(boardWin);
    const int maxX = getmaxx(boardWin);

    const std::string title =
        " " + players[static_cast<std::size_t>(focusIndex)].name + "'s view ";
    mvwprintw(boardWin, 1, 3, "%s", clipText(title, std::max(0, maxX - 6)).c_str());

    const std::string statusLine =
        players[static_cast<std::size_t>(focusIndex)].name + " at Space " +
        std::to_string(centerTile) + " - " + getTileDisplayName(tileAt(centerTile));
    mvwprintw(boardWin, 2, centeredX(1, maxX - 2, static_cast<int>(clipText(statusLine, maxX - 4).size())),
              "%s", clipText(statusLine, maxX - 4).c_str());

    const int viewportWidth = (VIEW_COLS * TILE_W) + ((VIEW_COLS - 1) * TILE_GAP_X) + 2;
    const int viewportHeight = (VIEW_ROWS * TILE_H) + ((VIEW_ROWS - 1) * TILE_GAP_Y) + 2;
    const int viewportLeft = std::max(1, (maxX - viewportWidth) / 2);
    const int viewportTop = 4;
    const int viewportBottom = std::min(maxY - 2, viewportTop + viewportHeight - 1);

    if (hasColor) {
        wattron(boardWin, COLOR_PAIR(8) | A_BOLD);
    }
    drawBoxAtSafe(boardWin, viewportTop, viewportLeft, viewportBottom - viewportTop + 1, viewportWidth);
    if (hasColor) {
        wattroff(boardWin, COLOR_PAIR(8) | A_BOLD);
    }

    const int tileStartLeft = viewportLeft + 1;
    const int tileStartTop = viewportTop + 1;
    const int centerCol = center.x;
    const int centerRow = center.y;
    const std::vector<std::pair<int, int> > connections = boardConnections(tiles);
    const std::set<int> reachable = reachableTiles(tiles);
    const std::set<int> visibleTrail =
        buildVisibleTrail(tiles, players[static_cast<std::size_t>(focusIndex)], focusTile);

    for (std::size_t i = 0; i < connections.size(); ++i) {
        if (reachable.count(connections[i].first) == 0 || reachable.count(connections[i].second) == 0) {
            continue;
        }
        if (visibleTrail.count(connections[i].first) == 0 || visibleTrail.count(connections[i].second) == 0) {
            continue;
        }
        const Tile& from = tiles[static_cast<std::size_t>(connections[i].first)];
        const Tile& to = tiles[static_cast<std::size_t>(connections[i].second)];
        const int fromDeltaCol = from.x - centerCol;
        const int fromDeltaRow = from.y - centerRow;
        const int toDeltaCol = to.x - centerCol;
        const int toDeltaRow = to.y - centerRow;
        if (fromDeltaCol < -2 || fromDeltaCol > 2 || fromDeltaRow < -1 || fromDeltaRow > 1 ||
            toDeltaCol < -2 || toDeltaCol > 2 || toDeltaRow < -1 || toDeltaRow > 1) {
            continue;
        }

        const int fromLeft = tileStartLeft + (fromDeltaCol + 2) * (TILE_W + TILE_GAP_X);
        const int fromTop = tileStartTop + (fromDeltaRow + 1) * (TILE_H + TILE_GAP_Y);
        const int toLeft = tileStartLeft + (toDeltaCol + 2) * (TILE_W + TILE_GAP_X);
        const int toTop = tileStartTop + (toDeltaRow + 1) * (TILE_H + TILE_GAP_Y);
        int startY = fromTop + (TILE_H / 2);
        int startX = fromLeft + (TILE_W / 2);
        int endY = toTop + (TILE_H / 2);
        int endX = toLeft + (TILE_W / 2);

        if (to.x > from.x) {
            startX = fromLeft + TILE_W - 1;
            endX = toLeft;
        } else if (to.x < from.x) {
            startX = fromLeft;
            endX = toLeft + TILE_W - 1;
        }

        if (to.y > from.y) {
            startY = fromTop + TILE_H - 1;
            endY = toTop;
        } else if (to.y < from.y) {
            startY = fromTop;
            endY = toTop + TILE_H - 1;
        }

        drawLineSegment(boardWin,
                        startY,
                        startX,
                        endY,
                        endX,
                        hasColor,
                        2,
                        maxY,
                        maxX);
    }

    for (int i = 0; i < TILE_COUNT; ++i) {
        if (reachable.count(i) == 0) {
            continue;
        }
        if (visibleTrail.count(i) == 0) {
            continue;
        }
        const Tile& tile = tiles[static_cast<std::size_t>(i)];
        const int deltaCol = tile.x - centerCol;
        const int deltaRow = tile.y - centerRow;
        if (deltaCol < -2 || deltaCol > 2 || deltaRow < -1 || deltaRow > 1) {
            continue;
        }

        const int col = deltaCol + 2;
        const int row = deltaRow + 1;
        const int tileLeft = tileStartLeft + col * (TILE_W + TILE_GAP_X);
        const int tileTop = tileStartTop + row * (TILE_H + TILE_GAP_Y);
        drawTileBox(boardWin,
                    tile,
                    players,
                    tileLeft,
                    tileTop,
                    i == focusTile,
                    hasColor);
    }

    wrefresh(boardWin);
}

void Board::render1860Selection(WINDOW* boardWin,
                                const std::vector<Player>& players,
                                int focusPlayerIndex,
                                int cursorTile,
                                const std::vector<int>& reachable,
                                int steps,
                                bool hasColor) const {
    werase(boardWin);
    drawBoxSafe(boardWin);
    const int focusIndex = std::max(0, std::min(focusPlayerIndex, static_cast<int>(players.size()) - 1));
    const int focusTile = players.empty() ? mode1860StartTileId() : players[static_cast<std::size_t>(focusIndex)].tile;
    const std::set<int> reachableSet(reachable.begin(), reachable.end());
    drawMode1860Board(boardWin,
                      mode1860Tiles,
                      players,
                      focusIndex,
                      focusTile,
                      cursorTile,
                      reachableSet,
                      steps,
                      hasColor);
    drawBoxSafe(boardWin);
    wrefresh(boardWin);
}
