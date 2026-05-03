#pragma once

#include <string>
#include <vector>

//Input: none (used as type specifier)
//Output: distinguishes between human and CPU players
//Purpose: defines player identity type
//Relation: used in Player struct and utility functions (playerTypeLabel, playerTypeFromText).
enum class PlayerType {
    Human,
    CPU
};
//Input: none (used as type specifier)
//Output: defines CPU difficulty level
//Purpose: categorizes CPU skill level
//Relation: used in Player struct and utility functions (cpuDifficultyLabel, cpuDifficultyFromText).
enum class CpuDifficulty {
    Easy,
    Normal,
    Hard
};

//Fields: (Identity): name, token, type, cpuDifficulty, (Position): tile, turnsTaken, skipNextTurn, movement penalties, (Economy): cash, salary, loans, investedNumber, investPayout, insuranceUses, (Life events): married, kids, collegeGraduate, usedNightSchool, hasHouse, houseName, houseValue, finalHouseSaleValue, (Retirement): retirementPlace, retirementBonus, retirementHome, retired, (Cards): actionCards, petCards, shieldCards
//        (Choices): startChoice, familyChoice, riskChoice, hasFamilyPath, familyBabyEventsRemaining, (Status effects): sabotageDebt, sabotageCooldown, itemDisableTurns, salary/movement penalties
//Purpose: represents a complete player state in the game, including identity, assets, progress, and modifiers
//Relation: Used by utility functions (tokenForName, totalWorth), Interacts with Bank (loans, cash), Rules (toggles, components), and Minigames (results tied to player state)
struct Player {
    std::string name;
    char token = 'A';
    int tile = 0;
    int cash = 0;
    std::string job;
    int salary = 0;
    bool married = false;
    int kids = 0;
    bool collegeGraduate = false;
    bool usedNightSchool = false;
    bool hasHouse = false;
    std::string houseName;
    int houseValue = 0;
    int loans = 0;
    int investedNumber = 0;
    int investPayout = 0;
    int spinToWinTokens = 0;
    int retirementPlace = 0;
    int retirementBonus = 0;
    int finalHouseSaleValue = 0;
    std::string retirementHome;
    std::vector<std::string> actionCards;
    std::vector<std::string> petCards;
    bool retired = false;
    int startChoice = -1;
    int familyChoice = -1;
    int riskChoice = -1;
    PlayerType type = PlayerType::Human;
    CpuDifficulty cpuDifficulty = CpuDifficulty::Normal;
    int turnsTaken = 0;
    int sabotageDebt = 0;
    int shieldCards = 0;
    int insuranceUses = 0;
    bool skipNextTurn = false;
    int movementPenaltyTurns = 0;
    int movementPenaltyPercent = 0;
    int salaryReductionTurns = 0;
    int salaryReductionPercent = 0;
    int sabotageCooldown = 0;
    int itemDisableTurns = 0;
    bool hasFamilyPath;
    int familyBabyEventsRemaining;
};

//Input: player name, fallback index
//Output: single character token
//Purpose: generates a short identifier for player
//Relation: used to label players in UI/game state.
char tokenForName(const std::string& name, int index);

//Input: Player object
//Output: integer net worth
//Purpose: calculates player’s final score based on assets and debts
//Relation: used at retirement or game end to determine winner.
int totalWorth(const Player& player);

//Input: PlayerType enum
//Output: string "CPU" or "Human"
//Purpose: converts enum to readable label
//Relation: used in UI and logs.
std::string playerTypeLabel(PlayerType type);

//Input: CpuDifficulty enum
//Output: string "Easy", "Normal", "Hard"
//Purpose: converts enum to readable label
//Relation: used in menus or debugging.
std::string cpuDifficultyLabel(CpuDifficulty difficulty);

//Input: text string
//Output: PlayerType::CPU if starts with 'C', else Human
//Purpose: parses text into player type
//Relation: supports configuration or command-line parsing.
PlayerType playerTypeFromText(const std::string& text);

//Input: text string
//Output: difficulty enum (Easy, Hard, Normal)
//Purpose: parses text into CPU difficulty level
//Relation: supports configuration or command-line parsing.
CpuDifficulty cpuDifficultyFromText(const std::string& text);
