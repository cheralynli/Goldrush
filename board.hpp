#pragma once
#include <vector>
#include <ncurses.h>
#include "Space.hpp"
#include "Player.hpp"

class Board {
    std::vector<Space> spaces;
    struct Coord { int y; int x; };
    std::vector<Coord> path;
    int width = 0;
    int height = 0;

public:
    void loadBoard();
    void display(WINDOW* win, const std::vector<Player>& players);
    Space getSpace(int position) const;
    int size() const { return static_cast<int>(spaces.size()); }
};
