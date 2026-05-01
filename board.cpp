#include "board.hpp"

#include <algorithm>
#include <array>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "ui.h"

namespace {

const short PAIR_TILE_BLACK_BG = 60;
const short PAIR_UI_YELLOW_BG = 61;
const short PAIR_GREEN_BG = 62;
const short PAIR_MAGENTA_BG = 63;
const short PAIR_CYAN_BG = 64;
const short PAIR_RED_BG = 65;
const short PAIR_PURPLE_BG = 66;

struct GridPoint {
    int row;
    int col;
};

const std::array<GridPoint, 15> kBlackTiles = {{
    {9, 2}, {9, 6}, {8, 0}, {8, 8}, {6, 2},
    {6, 6}, {5, 0}, {5, 4}, {5, 8}, {4, 2},
    {4, 6}, {3, 0}, {1, 2}, {0, 0}, {0, 4}
}};

int gCellH = CELL_H;
int gCellW = CELL_W;

int tileIdFor(int row, int col) {
    return row * BOARD_GRID_COLS + col;
}

std::pair<int, int> chooseCellSize(int availableHeight, int availableWidth) {
    const std::array<std::pair<int, int>, 3> presets = {{
        {CELL_H, CELL_W},
        {4, 8},
        {5, 10}
    }};

    for (std::size_t i = 0; i < presets.size(); ++i) {
        const int cellH = presets[i].first;
        const int cellW = presets[i].second;
        const int boardH = BOARD_ROWS * cellH + 1;
        const int boardW = BOARD_COLS * cellW + 1;
        if (boardH <= availableHeight && boardW <= availableWidth) {
            return presets[i];
        }
    }

    return presets.back();
}

bool pointMatches(const GridPoint& point, int row, int col) {
    return point.row == row && point.col == col;
}

bool isInsideWindow(WINDOW* win, int y, int x) {
    if (!win) {
        return false;
    }
    int maxY = 0;
    int maxX = 0;
    getmaxyx(win, maxY, maxX);
    return y >= 0 && y < maxY && x >= 0 && x < maxX;
}

void safeAddCh(WINDOW* win, int y, int x, chtype ch) {
    if (isInsideWindow(win, y, x)) {
        mvwaddch(win, y, x, ch);
    }
}

void fillRect(WINDOW* win, int y, int x, int height, int width, int colorPair) {
    if (!win) {
        return;
    }
    for (int dy = 0; dy < height; ++dy) {
        for (int dx = 0; dx < width; ++dx) {
            safeAddCh(win, y + dy, x + dx, ' ' | COLOR_PAIR(colorPair));
        }
    }
}

void drawContinuousGrid(WINDOW* win, int top, int left, int rows, int cols, int cellH, int cellW, int colorPair) {
    if (!win) {
        return;
    }
    const chtype borderAttr = COLOR_PAIR(colorPair) | A_BOLD;
    const int gridW = cols * cellW + 1;
    const int gridH = rows * cellH + 1;

    for (int row = 0; row <= rows; ++row) {
        const int y = top + (row * cellH);
        for (int dx = 0; dx < gridW; ++dx) {
            safeAddCh(win, y, left + dx, ACS_HLINE | borderAttr);
        }
    }

    for (int col = 0; col <= cols; ++col) {
        const int x = left + (col * cellW);
        for (int dy = 0; dy < gridH; ++dy) {
            safeAddCh(win, top + dy, x, ACS_VLINE | borderAttr);
        }
    }

    for (int row = 0; row <= rows; ++row) {
        const int y = top + (row * cellH);
        for (int col = 0; col <= cols; ++col) {
            const int x = left + (col * cellW);
            chtype corner = ACS_PLUS;
            if (row == 0 && col == 0) corner = ACS_ULCORNER;
            else if (row == 0 && col == cols) corner = ACS_URCORNER;
            else if (row == rows && col == 0) corner = ACS_LLCORNER;
            else if (row == rows && col == cols) corner = ACS_LRCORNER;
            else if (row == 0) corner = ACS_TTEE;
            else if (row == rows) corner = ACS_BTEE;
            else if (col == 0) corner = ACS_LTEE;
            else if (col == cols) corner = ACS_RTEE;
            safeAddCh(win, y, x, corner | borderAttr);
        }
    }
}

void fillInnerRect(WINDOW* win, int y, int x, int height, int width, int colorPair) {
    if (!win || height <= 0 || width <= 0) {
        return;
    }
    fillRect(win, y, x, height, width, colorPair);
}

void drawOutlineRect(WINDOW* win, int y, int x, int height, int width, int colorPair) {
    if (!win || height < 2 || width < 2) {
        return;
    }
    const chtype edge = ' ' | COLOR_PAIR(colorPair);
    for (int dx = 0; dx < width; ++dx) {
        safeAddCh(win, y, x + dx, edge);
        safeAddCh(win, y + height - 1, x + dx, edge);
    }
    for (int dy = 0; dy < height; ++dy) {
        safeAddCh(win, y + dy, x, edge);
        safeAddCh(win, y + dy, x + width - 1, edge);
    }
}

int labelColorForTile(int tileType) {
    switch (tileType) {
        case TILE_COLLEGE:
            return GOLDRUSH_TILE_WHITE;
        case TILE_CAREER:
        case TILE_GRADUATION:
            return GOLDRUSH_TILE_WHITE;
        case TILE_MARRIAGE:
        case TILE_RISKY:
            return GOLDRUSH_TILE_WHITE;
        case TILE_FAMILY:
        case TILE_BLACK:
            return GOLDRUSH_TILE_WHITE;
        case TILE_CAREER_2:
        case TILE_SAFE:
            return GOLDRUSH_TILE_WHITE;
        case TILE_START:
        case TILE_RETIREMENT:
            return GOLDRUSH_GOLD_SAND;
        default:
            return GOLDRUSH_GOLD_SAND;
    }
}

int tileTypeAtGameCoord(int gameRow, int gameCol) {
    if (gameRow == 0 && gameCol == 0) {
        return TILE_START;
    }
    if (gameRow == 0 && gameCol == 4) {
        return TILE_COLLEGE;
    }
    if (gameRow == 0 && gameCol == 8) {
        return TILE_CAREER;
    }
    if (gameRow == 2 && gameCol == 4) {
        return TILE_GRADUATION;
    }
    if (gameRow == 4 && gameCol == 10) {
        return TILE_MARRIAGE;
    }
    if (gameRow == 7 && gameCol == 4) {
        return TILE_FAMILY;
    }
    if (gameRow == 7 && gameCol == 8) {
        return TILE_CAREER_2;
    }
    if (gameRow == 9 && gameCol == 6) {
        return TILE_SAFE;
    }
    if (gameRow == 9 && gameCol == 8) {
        return TILE_RISKY;
    }
    if (gameRow == 10 && gameCol == 10) {
        return TILE_RETIREMENT;
    }
    for (std::size_t i = 0; i < kBlackTiles.size(); ++i) {
        if (pointMatches(kBlackTiles[i], gameRow, gameCol)) {
            return TILE_BLACK;
        }
    }
    return TILE_EMPTY;
}

void drawCenteredMarker(WINDOW* win,
                        int gameRow,
                        int gameCol,
                        int startY,
                        int startX,
                        char marker,
                        int colorPair) {
    const int screenRow = game_to_screen_row(gameRow);
    const int y = startY + (screenRow * gCellH);
    const int x = startX + (gameCol * gCellW);
    const int centerY = y + (gCellH / 2);
    const int centerX = x + (gCellW / 2);

    wattron(win, COLOR_PAIR(colorPair) | A_BOLD);
    safeAddCh(win, centerY, centerX, marker);
    wattroff(win, COLOR_PAIR(colorPair) | A_BOLD);
}

}  // namespace

std::string boardViewModeName(BoardViewMode mode) {
    return mode == BoardViewMode::ClassicFull ? "classic-full" : "follow-camera";
}

BoardViewMode boardViewModeFromName(const std::string& name) {
    return name == "classic-full" ? BoardViewMode::ClassicFull : BoardViewMode::ClassicFull;
}

void init_board_colors() {
    if (!has_colors()) {
        return;
    }

    start_color();
#ifdef NCURSES_VERSION
    use_default_colors();
#endif

    if (can_change_color()) {
        init_color(30, 1000, 910, 260);
        init_color(31, 820, 560, 120);
    }

    const short lightYellow = can_change_color() ? 30 : COLOR_YELLOW;
    const short darkYellow = can_change_color() ? 31 : COLOR_RED;
    const short gold = lightYellow;
    const short forest = COLOR_GREEN;
    const short mauve = COLOR_MAGENTA;
    const short steel = COLOR_CYAN;
    const short terra = COLOR_RED;

    init_pair(GOLDRUSH_BOARD_LIGHT, COLOR_BLACK, lightYellow);
    init_pair(GOLDRUSH_BOARD_DARK, COLOR_BLACK, darkYellow);
    init_pair(GOLDRUSH_GOLD_SAND, COLOR_YELLOW, COLOR_BLACK);
    init_pair(GOLDRUSH_GOLD_TERRA, COLOR_RED, COLOR_BLACK);
    init_pair(GOLDRUSH_TILE_CAREER, COLOR_CYAN, COLOR_BLACK);
    init_pair(GOLDRUSH_BLACK_FOREST, COLOR_GREEN, COLOR_BLACK);
    init_pair(GOLDRUSH_BLACK_TERRA, COLOR_RED, COLOR_BLACK);
    init_pair(GOLDRUSH_PLAYER_ONE, COLOR_YELLOW, COLOR_BLACK);
    init_pair(GOLDRUSH_PLAYER_TWO, COLOR_GREEN, COLOR_BLACK);
    init_pair(GOLDRUSH_PLAYER_THREE, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(GOLDRUSH_PLAYER_FOUR, COLOR_BLUE, COLOR_BLACK);
    init_pair(GOLDRUSH_TILE_WHITE, COLOR_WHITE, COLOR_BLACK);
    init_pair(GOLDRUSH_CHARCOAL_BLACK, COLOR_WHITE, COLOR_BLACK);
    init_pair(PAIR_TILE_BLACK_BG, COLOR_WHITE, COLOR_BLACK);
    init_pair(PAIR_UI_YELLOW_BG, COLOR_BLACK, gold);
    init_pair(PAIR_GREEN_BG, COLOR_BLACK, forest);
    init_pair(PAIR_MAGENTA_BG, COLOR_BLACK, mauve);
    init_pair(PAIR_CYAN_BG, COLOR_BLACK, steel);
    init_pair(PAIR_RED_BG, COLOR_BLACK, terra);
    init_pair(PAIR_PURPLE_BG, COLOR_BLACK, mauve);
}

int game_to_screen_row(int game_row) {
    return (BOARD_ROWS - 1) - game_row;
}

void draw_tile(WINDOW *win, int screen_row, int screen_col, int tile_type, int start_y, int start_x) {
    if (!win) {
        return;
    }

    const int y = start_y + (screen_row * gCellH);
    const int x = start_x + (screen_col * gCellW);
    const bool isYellowCell = ((screen_row + screen_col) % 2) == 1;

    if (isYellowCell) {
        fillInnerRect(win, y + 1, x + 1, std::max(1, gCellH - 1), std::max(1, gCellW - 1), PAIR_UI_YELLOW_BG);
        return;
    }

    const int specialPair =
        tile_type == TILE_COLLEGE ? PAIR_CYAN_BG :
        tile_type == TILE_CAREER ? PAIR_PURPLE_BG :
        tile_type == TILE_MARRIAGE ? PAIR_RED_BG :
        tile_type == TILE_FAMILY ? PAIR_MAGENTA_BG :
        tile_type == TILE_CAREER_2 ? PAIR_GREEN_BG :
        tile_type == TILE_SAFE ? PAIR_GREEN_BG :
        tile_type == TILE_RISKY ? PAIR_RED_BG :
        tile_type == TILE_GRADUATION ? PAIR_CYAN_BG :
        0;

    const std::string label =
        tile_type == TILE_START ? "START" :
        tile_type == TILE_RETIREMENT ? "END" :
        tile_type == TILE_COLLEGE ? "C" :
        tile_type == TILE_CAREER ? "CA" :
        tile_type == TILE_MARRIAGE ? "M" :
        tile_type == TILE_FAMILY ? "F" :
        tile_type == TILE_CAREER_2 ? "W" :
        tile_type == TILE_SAFE ? "$" :
        tile_type == TILE_RISKY ? "!" :
        tile_type == TILE_GRADUATION ? "G" :
        "";

    const int marginY = std::min(1, std::max(0, (gCellH - 2) / 2));
    const int marginX = std::min(1, std::max(0, (gCellW - 4) / 2));
    const int innerY = y + 1 + marginY;
    const int innerX = x + 1 + marginX;
    const int innerH = std::max(2, gCellH - 2 - (2 * marginY));
    const int innerW = std::max(4, gCellW - 2 - (2 * marginX));

    if (specialPair != 0) {
        drawOutlineRect(win, innerY, innerX, innerH, innerW, specialPair);
    }

    if (!label.empty()) {
        const int labelY = y + (gCellH / 2);
        const int labelX = x + std::max(1, (gCellW - static_cast<int>(label.size())) / 2);
        const int labelPair = labelColorForTile(tile_type);
        wattron(win, COLOR_PAIR(labelPair) | A_BOLD);
        mvwprintw(win, labelY, labelX, "%s", label.c_str());
        wattroff(win, COLOR_PAIR(labelPair) | A_BOLD);
    }
}

void draw_board(WINDOW *win, int start_y, int start_x) {
    if (!win) {
        return;
    }

    for (int gameRow = 0; gameRow < BOARD_ROWS; ++gameRow) {
        const int screenRow = game_to_screen_row(gameRow);
        for (int gameCol = 0; gameCol < BOARD_COLS; ++gameCol) {
            draw_tile(win,
                      screenRow,
                      gameCol,
                      tileTypeAtGameCoord(gameRow, gameCol),
                      start_y,
                      start_x);
        }
    }
}

void draw_player(WINDOW *win, int game_row, int game_col, int start_y, int start_x) {
    if (!win) {
        return;
    }
    drawCenteredMarker(win, game_row, game_col, start_y, start_x, '@', GOLDRUSH_TILE_WHITE);
}

Board::Board() {
    initTiles();
    initRegions();
}

const Tile& Board::tileAt(int id) const {
    return tiles.at(static_cast<std::size_t>(id));
}

bool Board::isStopSpace(const Tile& tile) const {
    return tile.kind != TILE_EMPTY;
}

std::string Board::regionNameForTile(int tileIndex) const {
    const Tile& tile = tileAt(tileIndex);
    switch (tile.kind) {
        case TILE_START:
            return "Start";
        case TILE_COLLEGE:
        case TILE_GRADUATION:
            return "College";
        case TILE_CAREER:
        case TILE_CAREER_2:
            return "Career";
        case TILE_MARRIAGE:
            return "Marriage";
        case TILE_FAMILY:
            return "Family";
        case TILE_SAFE:
            return "Safe";
        case TILE_RISKY:
            return "Risk";
        case TILE_RETIREMENT:
            return "Retirement";
        case TILE_BLACK:
            return "Black Tile";
        case TILE_EMPTY:
        default:
            return "Open Trail";
    }
}

std::vector<std::string> Board::tutorialLegend() const {
    return std::vector<std::string>{
        "START: begin here and collect $10,000.",
        "COLLEGE: pay $10,000 tuition and take a $20,000 loan.",
        "CAREER: start working with a $3,000 salary.",
        "GRADUATION: upgrade to a better $6,000-$8,000 job.",
        "MARRIAGE: pay $5,000 and gain a spouse.",
        "FAMILY: gain 1 kid and pay $2,000 per kid.",
        "WORK: salary promotion worth +$5,000.",
        "SAFE: guaranteed +$3,000.",
        "RISK: 50/50 for +$15,000 or -$10,000.",
        "RETIREMENT: first player here ends the game.",
        "c tiles: launch Pong, Minesweeper, or Spaceship Shooter.",
        "Teetotum: spin 1-6, then choose a valid direction."
    };
}

bool Board::isInsideGrid(int row, int col) const {
    return row >= 0 && row < BOARD_GRID_ROWS && col >= 0 && col < BOARD_GRID_COLS;
}

int Board::tileIdAt(int row, int col) const {
    return isInsideGrid(row, col) ? tileIdFor(row, col) : -1;
}

void Board::initTiles() {
    tiles.clear();
    tiles.resize(TILE_COUNT);
    for (int row = 0; row < BOARD_GRID_ROWS; ++row) {
        for (int col = 0; col < BOARD_GRID_COLS; ++col) {
            const int id = tileIdFor(row, col);
            Tile& tile = tiles[static_cast<std::size_t>(id)];
            tile.id = id;
            tile.y = row;
            tile.x = col;
            tile.label = "";
            tile.kind = TILE_EMPTY;
            tile.next = (id + 1 < TILE_COUNT) ? id + 1 : -1;
            tile.altNext = -1;
            tile.value = 0;
            tile.stop = false;
        }
    }

    auto setTile = [this](int row, int col, TileKind kind, const std::string& label, int value = 0) {
        Tile& tile = tiles[static_cast<std::size_t>(tileIdFor(row, col))];
        tile.kind = kind;
        tile.label = label;
        tile.value = value;
        tile.stop = kind != TILE_EMPTY;
    };

    setTile(0, 10, TILE_RETIREMENT, "RETIRE", 0);
    setTile(1, 6, TILE_SAFE, "SAFE", 3000);
    setTile(1, 8, TILE_RISKY, "RISK", 0);
    setTile(3, 4, TILE_FAMILY, "FAMILY", 2000);
    setTile(3, 8, TILE_CAREER_2, "WORK", 5000);
    setTile(6, 10, TILE_MARRIAGE, "MARRIAGE", 5000);
    setTile(8, 4, TILE_GRADUATION, "GRAD", 0);
    setTile(10, 0, TILE_START, "START", 10000);
    setTile(10, 4, TILE_COLLEGE, "COLLEGE", 10000);
    setTile(10, 8, TILE_CAREER, "CAREER", 3000);

    for (std::size_t i = 0; i < kBlackTiles.size(); ++i) {
        setTile(kBlackTiles[i].row, kBlackTiles[i].col, TILE_BLACK, "c", 1);
    }
}

void Board::initRegions() {
    regions.clear();
    regions.push_back({"Opening", tileIdFor(0, 0), tileIdFor(3, 10)});
    regions.push_back({"Middle Years", tileIdFor(4, 0), tileIdFor(7, 10)});
    regions.push_back({"Late Game", tileIdFor(8, 0), tileIdFor(10, 10)});
}

void Board::render(WINDOW* boardWin,
                   const std::vector<Player>& players,
                   int focusPlayerIndex,
                   int highlightedTile,
                   bool hasColor,
                   BoardViewMode /*viewMode*/) const {
    if (!boardWin) {
        return;
    }

    int height = 0;
    int width = 0;
    getmaxyx(boardWin, height, width);
    werase(boardWin);
    drawBoxSafe(boardWin);

    if (hasColor) {
        init_board_colors();
    }

    const int contentTop = 1 + BOARD_PAD_Y;
    const int contentBottomMargin = 1 + BOARD_PAD_Y;
    const int contentSideMargin = 1 + BOARD_PAD_X;
    const int availableHeight = std::max(1, height - contentTop - contentBottomMargin);
    const int availableWidth = std::max(1, width - (2 * contentSideMargin));
    const std::pair<int, int> cellSize = chooseCellSize(availableHeight, availableWidth);
    gCellH = cellSize.first;
    gCellW = cellSize.second;

    const int boardWidth = BOARD_COLS * gCellW + 1;
    const int boardHeight = BOARD_ROWS * gCellH + 1;
    const int startY = contentTop;
    const int startX = contentSideMargin;

    fillRect(boardWin, startY, startX, boardHeight, boardWidth, PAIR_TILE_BLACK_BG);
    drawContinuousGrid(boardWin, startY, startX, BOARD_ROWS, BOARD_COLS, gCellH, gCellW, GOLDRUSH_GOLD_SAND);
    draw_board(boardWin, startY, startX);

    if (highlightedTile >= 0 && highlightedTile < TILE_COUNT) {
        const Tile& tile = tileAt(highlightedTile);
        const int y = startY + (game_to_screen_row(tile.y) * gCellH);
        const int x = startX + (tile.x * gCellW);
        if (hasColor) {
            wattron(boardWin, COLOR_PAIR(GOLDRUSH_TILE_WHITE) | A_BOLD);
        }
        safeAddCh(boardWin, y, x, ACS_DIAMOND);
        if (hasColor) {
            wattroff(boardWin, COLOR_PAIR(GOLDRUSH_TILE_WHITE) | A_BOLD);
        }
    }

    std::map<std::pair<int, int>, std::vector<std::size_t> > occupantMap;
    for (std::size_t i = 0; i < players.size(); ++i) {
        if (players[i].tile < 0 || players[i].tile >= TILE_COUNT) {
            continue;
        }
        const Tile& tile = tileAt(players[i].tile);
        occupantMap[std::make_pair(tile.y, tile.x)].push_back(i);
    }

    for (std::map<std::pair<int, int>, std::vector<std::size_t> >::const_iterator it = occupantMap.begin();
         it != occupantMap.end();
         ++it) {
        const int gameRow = it->first.first;
        const int gameCol = it->first.second;
        const std::vector<std::size_t>& indices = it->second;
        if (indices.empty()) {
            continue;
        }

        char marker = '@';
        int markerPair = GOLDRUSH_TILE_WHITE;
        if (indices.size() == 1U) {
            const std::size_t playerIndex = indices.front();
            marker = players[playerIndex].token;
            markerPair = static_cast<int>(playerIndex) == focusPlayerIndex ? GOLDRUSH_TILE_WHITE :
                ui_player_color_pair(static_cast<int>(playerIndex));
        }
        drawCenteredMarker(boardWin, gameRow, gameCol, startY, startX, marker, markerPair);
    }

    wrefresh(boardWin);
}
