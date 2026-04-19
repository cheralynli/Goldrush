#include "Board.hpp"

Board::Board() {
    initTiles();
}

const Tile& Board::tileAt(int id) const {
    return tiles[id];
}

void Board::initTiles() {
    tiles.resize(TILE_COUNT);
    for (int i = 0; i < TILE_COUNT; ++i) {
        tiles[i].id = i;
        tiles[i].y = 1;
        tiles[i].x = 2;
        tiles[i].label = "  ";
        tiles[i].kind = TILE_EMPTY;
        tiles[i].next = (i < TILE_COUNT - 1) ? i + 1 : -1;
        tiles[i].altNext = -1;
        tiles[i].value = 0;
    }

    for (int i = 0; i <= 11; ++i) {
        tiles[i].y = 1;
        tiles[i].x = 16 + (i * 4);
    }

    tiles[12].y = 4;
    tiles[12].x = 38;

    for (int i = 13; i <= 24; ++i) {
        tiles[i].y = 7;
        tiles[i].x = 2 + ((i - 13) * 4);
    }

    for (int i = 25; i <= 37; ++i) {
        tiles[i].y = 10;
        tiles[i].x = 24 + ((i - 25) * 4);
    }

    for (int i = 38; i <= 48; ++i) {
        tiles[i].y = 13;
        tiles[i].x = 18 + ((i - 38) * 4);
    }

    for (int i = 49; i <= 58; ++i) {
        tiles[i].y = 16;
        tiles[i].x = 20 + ((i - 49) * 4);
    }

    for (int i = 59; i <= 72; ++i) {
        tiles[i].y = 19;
        tiles[i].x = 2 + ((i - 59) * 4);
    }

    for (int i = 73; i <= 86; ++i) {
        tiles[i].y = 22;
        tiles[i].x = 22 + ((i - 73) * 4);
    }

    tiles[87].y = 25;
    tiles[87].x = 36;
    tiles[88].y = 25;
    tiles[88].x = 40;

    for (int i = 0; i <= 9; ++i) {
        tiles[i].label = "BK";
        tiles[i].kind = TILE_BLACK;
        tiles[i].value = 1;
    }
    tiles[10].label = "ST";
    tiles[10].kind = TILE_START;
    tiles[11].label = "BK";
    tiles[11].kind = TILE_BLACK;
    tiles[11].value = 1;
    tiles[12].label = "SP";
    tiles[12].kind = TILE_SPLIT_START;
    tiles[12].next = 13;
    tiles[12].altNext = 25;

    tiles[13].label = "COL";
    tiles[13].kind = TILE_COLLEGE;
    for (int i = 14; i <= 24; ++i) {
        tiles[i].label = "BK";
        tiles[i].kind = TILE_BLACK;
        tiles[i].value = (i >= 20) ? 2 : 1;
    }
    tiles[24].next = 38;

    tiles[25].label = "CAR";
    tiles[25].kind = TILE_CAREER;
    for (int i = 26; i <= 37; ++i) {
        tiles[i].label = "BK";
        tiles[i].kind = TILE_BLACK;
        tiles[i].value = (i >= 33) ? 2 : 1;
    }
    tiles[37].next = 38;

    tiles[38].label = "GRD";
    tiles[38].kind = TILE_GRADUATION;
    for (int i = 39; i <= 48; ++i) {
        tiles[i].label = "BK";
        tiles[i].kind = TILE_BLACK;
        tiles[i].value = 2;
    }
    tiles[41].label = "PAY";
    tiles[41].kind = TILE_PAYDAY;
    tiles[41].value = 4000;
    tiles[44].label = "WED";
    tiles[44].kind = TILE_MARRIAGE;
    tiles[47].label = "PAY";
    tiles[47].kind = TILE_PAYDAY;
    tiles[47].value = 5000;

    for (int i = 49; i <= 57; ++i) {
        tiles[i].label = "BK";
        tiles[i].kind = TILE_BLACK;
        tiles[i].value = 2;
    }
    tiles[58].label = "SP";
    tiles[58].kind = TILE_SPLIT_FAMILY;
    tiles[49].next = 50;
    tiles[58].next = 59;
    tiles[58].altNext = 73;

    for (int i = 59; i <= 72; ++i) {
        tiles[i].label = "BK";
        tiles[i].kind = TILE_BLACK;
        tiles[i].value = 2;
    }
    tiles[60].label = "3B";
    tiles[60].kind = TILE_BABY;
    tiles[60].value = 3;
    tiles[64].label = "2B";
    tiles[64].kind = TILE_BABY;
    tiles[64].value = 2;
    tiles[68].label = "HSE";
    tiles[68].kind = TILE_HOUSE;
    tiles[68].value = 100000;
    tiles[72].label = "1B";
    tiles[72].kind = TILE_BABY;
    tiles[72].value = 1;
    tiles[72].next = 87;

    for (int i = 73; i <= 86; ++i) {
        tiles[i].label = "BK";
        tiles[i].kind = TILE_BLACK;
        tiles[i].value = 2;
    }
    tiles[75].label = "PAY";
    tiles[75].kind = TILE_PAYDAY;
    tiles[75].value = 5000;
    tiles[79].label = "PRM";
    tiles[79].kind = TILE_CAREER_2;
    tiles[79].value = 3000;
    tiles[83].label = "PAY";
    tiles[83].kind = TILE_PAYDAY;
    tiles[83].value = 7000;
    tiles[86].label = "BK";
    tiles[86].value = 3;
    tiles[86].next = 87;

    tiles[87].label = "RT";
    tiles[87].kind = TILE_RETIREMENT;
    tiles[88].label = "RT";
    tiles[88].kind = TILE_RETIREMENT;
    tiles[87].next = 88;
    tiles[88].next = -1;
}

int Board::colorForTile(const Tile& tile) const {
    if (tile.kind == TILE_BLACK) return 6;
    if (tile.kind == TILE_START) return 3;
    if (tile.kind == TILE_SPLIT_START || tile.kind == TILE_SPLIT_FAMILY) return 4;
    if (tile.kind == TILE_COLLEGE) return 2;
    if (tile.kind == TILE_CAREER || tile.kind == TILE_CAREER_2) return 1;
    if (tile.kind == TILE_PAYDAY) return 7;
    if (tile.kind == TILE_BABY) return 3;
    if (tile.kind == TILE_HOUSE) return 2;
    if (tile.kind == TILE_GRADUATION) return 7;
    if (tile.kind == TILE_MARRIAGE) return 6;
    if (tile.kind == TILE_FAMILY) return 7;
    if (tile.kind == TILE_RETIREMENT) return 3;
    return 5;
}

void Board::drawTreeGuides(WINDOW* boardWin) const {
    mvwvline(boardWin, 2, 39, ACS_VLINE, 2);
    mvwaddch(boardWin, 4, 39, ACS_TTEE);

    mvwhline(boardWin, 6, 24, ACS_HLINE, 16);
    mvwvline(boardWin, 5, 24, ACS_VLINE, 2);
    mvwvline(boardWin, 5, 56, ACS_VLINE, 5);
    mvwaddch(boardWin, 6, 24, ACS_LTEE);
    mvwaddch(boardWin, 6, 56, ACS_RTEE);

    mvwvline(boardWin, 11, 39, ACS_VLINE, 2);
    mvwvline(boardWin, 14, 39, ACS_VLINE, 2);
    mvwaddch(boardWin, 13, 39, ACS_VLINE);

    mvwhline(boardWin, 18, 26, ACS_HLINE, 14);
    mvwvline(boardWin, 17, 26, ACS_VLINE, 2);
    mvwvline(boardWin, 17, 54, ACS_VLINE, 5);
    mvwaddch(boardWin, 18, 26, ACS_LTEE);
    mvwaddch(boardWin, 18, 54, ACS_RTEE);

    mvwvline(boardWin, 23, 39, ACS_VLINE, 2);
}

void Board::render(WINDOW* boardWin,
                   const std::vector<Player>& players,
                   bool hasColor) const {
    werase(boardWin);
    box(boardWin, 0, 0);
    drawTreeGuides(boardWin);

    for (int i = 0; i < TILE_COUNT; ++i) {
        const Tile& tile = tiles[i];
        int color = colorForTile(tile);
        mvwaddch(boardWin, tile.y, tile.x, '[');
        if (hasColor) wattron(boardWin, COLOR_PAIR(color));
        mvwprintw(boardWin, tile.y, tile.x + 1, "  ");
        if (hasColor) wattroff(boardWin, COLOR_PAIR(color));
        mvwaddch(boardWin, tile.y, tile.x + 3, ']');
        mvwprintw(boardWin, tile.y + 1, tile.x + 1, "%s", tile.label.c_str());
    }

    for (int i = 0; i < TILE_COUNT; ++i) {
        int slot = 0;
        for (size_t p = 0; p < players.size(); ++p) {
            if (players[p].tile != i) continue;
            if (slot >= 2) break;
            if (hasColor) wattron(boardWin, COLOR_PAIR(1 + static_cast<int>(p % 4)) | A_BOLD);
            mvwaddch(boardWin, tiles[i].y, tiles[i].x + 1 + slot, players[p].token);
            if (hasColor) wattroff(boardWin, COLOR_PAIR(1 + static_cast<int>(p % 4)) | A_BOLD);
            slot++;
        }
    }
    wrefresh(boardWin);
}
