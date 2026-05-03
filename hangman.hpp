#pragma once

#include <string>

//Fields:(bool won)true if the player successfully guessed the word, (int attemptsLeft)number of tries remaining (0–10), (int lettersGuessed)number of letter slots revealed, (bool abandoned)true if the player quit (Q key)
//Purpose:encapsulates the outcome of a Hangman minigame run
//Relation:returned by playHangmanMinigame to summarize player performance and game status.
struct HangmanResult {
    bool won;           // Did the player guess the word?
    int attemptsLeft;   // How many tries remaining (0-10)
    int lettersGuessed; // How many letter slots were revealed
    bool abandoned;     // Did player quit (Q key)?
};

//Input:(playerName)used in UI/status line for personalization, (hasColor)flag to enable colored ncurses output
//Output:HangmanResult (won, attempts left, letters guessed, abandoned flag)
//Purpose:runs the Hangman minigame loop and returns the result summary
//Relation:Uses HangmanResult to communicate results back to the caller, Internally handles word selection, input processing, letter reveal logic, and quit detection, Follows the same minigame pattern as Pong, Battleship, Minesweeper, and Memory Match.
HangmanResult playHangmanMinigame(const std::string& playerName, bool hasColor);
