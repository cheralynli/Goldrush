#pragma once

#include <string>

//Input: none 
//Output: holds the outcome of a Minesweeper minigame run
//Purpose: encapsulates the player’s performance and game state
//Relation: returned by playMinesweeperMinigame() to report results
struct MinesweeperResult {
    int safeTilesRevealed;
    bool hitBomb;
    bool timedOut;
    bool abandoned;
};

MinesweeperResult playMinesweeperMinigame(const std::string& playerName, bool hasColor);
