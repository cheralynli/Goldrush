#pragma once

#include <ncurses.h>
#include <string>
#include <vector>

#include "player.hpp"

#define BOARD_ROWS 11
#define BOARD_COLS 11
#define CELL_H 5
#define CELL_W 10
#define INNER_MARGIN_Y 1
#define INNER_MARGIN_X 2

static const int BOARD_GRID_COLS = BOARD_COLS;
static const int BOARD_GRID_ROWS = BOARD_ROWS;
static const int TILE_COUNT = BOARD_GRID_COLS * BOARD_GRID_ROWS;

enum TileKind {
    TILE_EMPTY,
    TILE_BLACK,
    TILE_START,
    TILE_SPLIT_START,
    TILE_COLLEGE,
    TILE_CAREER,
    TILE_GRADUATION,
    TILE_MARRIAGE,
    TILE_SPLIT_FAMILY,
    TILE_FAMILY,
    TILE_NIGHT_SCHOOL,
    TILE_SPLIT_RISK,
    TILE_SAFE,
    TILE_RISKY,
    TILE_SPIN_AGAIN,
    TILE_CAREER_2,
    TILE_PAYDAY,
    TILE_BABY,
    TILE_RETIREMENT,
    TILE_HOUSE
};

struct Tile {
    int id;
    int y;
    int x;
    std::string label;
    TileKind kind;
    int next;
    int altNext;
    int value;
    bool stop;
};

struct BoardRegion {
    std::string name;
    int startTileIndex;
    int endTileIndex;
};

enum class BoardViewMode {
    FollowCamera,
    ClassicFull
};

std::string boardViewModeName(BoardViewMode mode);
BoardViewMode boardViewModeFromName(const std::string& name);

// Standalone renderer helpers. These are intentionally simple so they can be
// called directly from an ncurses game loop without pulling in board logic.
void init_board_colors();
int game_to_screen_row(int game_row);
void draw_tile(WINDOW *win, int screen_row, int screen_col, int tile_type, int start_y, int start_x);
void draw_board(WINDOW *win, int start_y, int start_x);
void draw_player(WINDOW *win, int game_row, int game_col, int start_y, int start_x);

class Board {
public:
    Board();

    const Tile& tileAt(int id) const;
    bool isStopSpace(const Tile& tile) const;
    std::string regionNameForTile(int tileIndex) const;
    std::vector<std::string> tutorialLegend() const;
    void render(WINDOW* boardWin,
                const std::vector<Player>& players,
                int focusPlayerIndex,
                int highlightedTile,
                bool hasColor,
                BoardViewMode viewMode = BoardViewMode::ClassicFull) const;

    bool isInsideGrid(int row, int col) const;
    int tileIdAt(int row, int col) const;

private:
    std::vector<Tile> tiles;
    std::vector<BoardRegion> regions;

    void initTiles();
    void initRegions();
};
