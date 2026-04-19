#pragma once

#include <ncurses.h>
#include <string>
#include <vector>

#include "Player.hpp"

static const int TILE_COUNT = 89;

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
};

class Board {
public:
    Board();

    const Tile& tileAt(int id) const;
    void render(WINDOW* boardWin,
                const std::vector<Player>& players,
                bool hasColor) const;

private:
    std::vector<Tile> tiles;

    void initTiles();
    int colorForTile(const Tile& tile) const;
    void drawTreeGuides(WINDOW* boardWin) const;
    void drawBoardGrid(WINDOW* boardWin) const;
    void drawTile(WINDOW* boardWin, const Tile& tile, bool hasColor) const;
    void drawTokens(WINDOW* boardWin, const std::vector<Player>& players, int tileIndex, bool hasColor) const;
};
