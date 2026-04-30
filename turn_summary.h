#pragma once

#include <string>
#include <vector>

//Input: turn events
//Output: none
//Purpose: defines the structure of information needed to be reported in summary
//Relation: informs user of important events after turn is over
struct TurnSummary {
    std::string playerName;
    int turnNumber = 0;

    int moneyChange = 0;
    int loanChange = 0;
    int babyChange = 0;
    int petChange = 0;
    int investmentChange = 0;
    int shieldChange = 0;
    int insuranceChange = 0;

    bool gotMarried = false;
    bool jobChanged = false;
    bool houseChanged = false;

    std::string oldJob;
    std::string newJob;
    std::string oldHouse;
    std::string newHouse;

    std::vector<std::string> cardsGained;
    std::vector<std::string> cardsUsed;
    std::vector<std::string> minigameResults;
    std::vector<std::string> sabotageEvents;
    std::vector<std::string> importantEvents;
};

//Input: int amount, string unit
//Output: string formatted amounts
//Purpose: to display numbers to user on summary
//Relation: used to format changes before display
std::string formatMoney(int amount);
std::string formatSignedChange(int amount, const std::string& unit);

//Input: TurnSummary summary, bool hasColor
//Output: none
//Purpose: declares showTurnSummaryReport
//Relation: to display summary popup
void showTurnSummaryReport(const TurnSummary& summary, bool hasColor);
