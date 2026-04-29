#include <ncurses.h>

#include "game.hpp"
#include "ui.h"

int main() {
    //sets up ncurses environment, colors, and UI state
    initialize_game_ui(); 

    //constructs the Game object
    Game game; 
    //enters the main game loop
    game.run(); 

    //cleans up ncurses resources before exiting
    destroy_game_ui(); 
    return 0;
}
