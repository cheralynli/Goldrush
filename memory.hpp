#pragma once

#include <string>
#include <vector>

//Input: none 
//Output: holds the outcome of a Memory Match minigame run
//Purpose: summarize player?s performance and game state
//Relation: returned by playMemoryMatchMinigame func to report results
struct MemoryMatchResult {
    int pairsMatched;   // Number of pairs correctly matched (0-8)
    int livesRemaining; // Lives left (0-20)
    bool abandoned;     // Did player quit?
    bool won;           // Did they match all pairs?
};

//Input: playerName (string), hasColor (bool)
//Output: MemoryMatchResult struct (pairsMatched, livesRemaining, abandoned, won)
//Purpose: runs the Memory Match minigame loop, handles input, rendering, and game logic, then returns a summary of the player?s performance
//Relation: main entry point for the Memory Match minigame; integrates tutorial display,grid creation, shuffle logic, reveal/match mechanics, feedback banners, and result reporting
MemoryMatchResult playMemoryMatchMinigame(const std::string& playerName, bool hasColor);

//Input: unicodeSupported (bool flag, currently unused)
//Output: vector of strings representing symbols (A?H)
//Purpose: provides the set of symbols used for matching pairs
//Relation: used in shuffleGrid to populate the grid with pairs
std::vector<std::string> getMemoryMatchSymbols(bool unicodeSupported);
