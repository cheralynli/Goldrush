#include <ncurses.h>

#include "game.hpp"
#include "ui.h"

int main() {
    initialize_game_ui(); //sets up ncurses environment, colors, and UI state

    Game game; //constructs the Game object
    game.run(); //enters the main game loop

    destroy_game_ui(); //cleans up ncurses resources before exiting
    return 0;
}
