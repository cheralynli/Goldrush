#pragma once

#include <string>
#include <vector>

//Fields:
//(std::string date) date of game completion
//(std::string gameId) unique identifier for the game session
//(std::string winner) name of winning player
//(int winnerScore) final score of winner
//(int winnerCash) cash balance of winner
//(int winnerNetWorth) net worth of winner
//(int rounds) number of rounds played
//(std::vector<std::string> detailLines) additional recorded details
//(std::string mode) game mode (Relax, Life, Hell, Custom)
//(std::string settings) configuration summary string
//Purpose: encapsulates all recorded data for a completed game session
//Relation: used as the data model for persistence and display in history
struct CompletedGameEntry {
    std::string date;
    std::string gameId;
    std::string winner;
    int winnerScore = 0;
    int winnerCash = 0;
    int winnerNetWorth = 0;
    int rounds = 0;
    std::string players;
    std::string playerDetails;
    std::vector<std::string> detailLines;
    std::string mode;
    std::string settings;
};

//Input: completed game entry, error string reference
//Output: true if written successfully, false otherwise
//Purpose: appends entry to persistent log file
//Relation: called at end of game to save results.
bool appendCompletedGameHistory(const CompletedGameEntry& entry, std::string& error);

//Input: error string reference
//Output: vector of completed game entries
//Purpose: loads and parses log file into structured entries
//Relation: used to populate history screen.
std::vector<CompletedGameEntry> loadCompletedGameHistory(std::string& error);

//Input: color flag
//Output: ncurses popup with list of completed games
//Purpose: displays history list with navigation and detail view
//Relation: integrates with loadCompletedGameHistory and detail screen functions
void showCompletedGameHistoryScreen(bool hasColor);
