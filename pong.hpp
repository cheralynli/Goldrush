#pragma once

#include <string>
//Fields: (int playerScore)score achieved by the human player, (int cpuScore)score achieved by the CPU opponent, (bool abandoned)whether the player quit before completion
//Purpose: covers the outcome of a single-player Pong minigame run
//Relation: returned by playPongMinigame to summarize performance against CPU.
struct PongMinigameResult {
    int playerScore;
    int cpuScore;
    bool abandoned;
};
//Fields: (int winnerSide) 0 = left player, 1 = right player, -1 = none (no winner), (bool abandoned) whether the duel was quit before completion
//Purpose: encapsulates the outcome of a two-player Pong duel
//Relation: returned by playPongDuelMinigame to report duel results.
struct PongDuelResult {
    int winnerSide; // 0 = left, 1 = right, -1 = none
    bool abandoned;
};
//Input:playerName → used in UI/status line for personalization, hasColor → flag to enable colored ncurses output
//Output: PongMinigameResult (player score, CPU score, abandoned flag)
//Purpose: runs the single-player Pong minigame loop against CPU
//Relation: Uses PongMinigameResult to communicate results back to caller, Internally handles input, rendering, scoring, and quit detection. 
PongMinigameResult playPongMinigame(const std::string& playerName, bool hasColor);
//Input:(leftPlayerName)name of left-side player, (rightPlayerName)name of right-side player (or CPU if rightSideCpu true), (hasColor)flag to enable colored ncurses output, (rightSideCpu) determines if right player is CPU-controlled
//Output:PongDuelResult (winner side, abandoned flag)
//Purpose: runs a two-player Pong duel (human vs human or human vs CPU)
//Relation: Uses PongDuelResult to report duel outcome, Extends single-player logic to support multiplayer or CPU duel.
PongDuelResult playPongDuelMinigame(const std::string& leftPlayerName,
                                    const std::string& rightPlayerName,
                                    bool hasColor,
                                    bool rightSideCpu);
