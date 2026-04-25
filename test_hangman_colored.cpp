#include "hangman.hpp"
#include "ui.h"

int main() {
    initialize_game_ui();
    
    clear();
    mvprintw(1, 2, "Testing Hangman Game");
    mvprintw(2, 2, "Press any key to start...");
    refresh();
    getch();
    
    HangmanResult result = playHangmanMinigame("TESTER", has_colors());
    
    clear();
    mvprintw(5, 2, "=== HANGMAN RESULTS ===");
    mvprintw(7, 4, "Won: %s", result.won ? "YES" : "NO");
    mvprintw(8, 4, "Attempts left: %d", result.attemptsLeft);
    mvprintw(9, 4, "Abandoned: %s", result.abandoned ? "YES" : "NO");
    mvprintw(12, 2, "Press any key to exit...");
    refresh();
    getch();
    
    destroy_game_ui();
    return 0;
}