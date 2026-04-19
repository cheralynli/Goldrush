#include <ncurses.h>

#include "Game.hpp"
#include "ui.h"

int main() {
    initialize_game_ui();

    Game game;
    game.run();

    destroy_game_ui();
    return 0;
}
