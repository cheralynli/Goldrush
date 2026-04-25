#include <ncurses.h>
#include "memory.hpp"
#include "ui.h"

int main() {
    // Initialize full game UI (includes all colors)
    initialize_game_ui();
    
    clear();
    mvprintw(1, 2, "Testing Memory Match Game");
    mvprintw(2, 2, "Press any key to start...");
    refresh();
    getch();
    
    // Play the minigame
    MemoryMatchResult result = playMemoryMatchMinigame("TESTER", has_colors());
    
    // Show results
    clear();
    mvprintw(5, 2, "=== MEMORY MATCH RESULTS ===");
    mvprintw(7, 4, "Won: %s", result.won ? "YES" : "NO");
    mvprintw(8, 4, "Pairs matched: %d/8", result.pairsMatched);
    mvprintw(9, 4, "Lives remaining: %d", result.livesRemaining);
    mvprintw(10, 4, "Abandoned: %s", result.abandoned ? "YES" : "NO");
    mvprintw(12, 2, "Press any key to exit...");
    refresh();
    getch();
    
    // Clean up
    destroy_game_ui();
    return 0;
}