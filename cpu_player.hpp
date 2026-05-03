#pragma once

#include <string>
#include <vector>

#include "cards.hpp"
#include "player.hpp"
#include "random_service.hpp"
#include "rules.hpp"
#include "sabotage_card.h"

//Fields:(int score)simulated minigame score, (bool success)whether CPU achieved threshold for success, (std::string summary)human-readable performance summary
//Purpose: encapsulates CPU minigame outcomes
//Relation: returned by playBlackTileMinigame to summarize AI performance.
struct CpuMinigameResult {
    int score;
    bool success;
    std::string summary;
};

//Purpose: decision-making engine for CPU players
//Relation: integrates with Player, RuleSet, CareerCard, SabotageType, and RandomService to simulate CPU choices across routes, careers, minigames, and sabotage
class CpuController {
public:

    //Input: RNG service reference
    //Output: controller bound to randomness source
    //Purpose: initializes CPU decision-making with randomness support
    explicit CpuController(RandomService& random);

//Purpose: decides between college or career start path
//Relation: depends on difficulty and player’s cash/salary.
    int chooseStartRoute(const Player& player, const RuleSet& rules);

//Purpose: decides whether CPU takes family path
//Relation: depends on difficulty, kids, cash, and rules toggle
    int chooseFamilyRoute(const Player& player, const RuleSet& rules);

//Purpose: decides whether CPU takes risky route
//Relation: depends on difficulty, cash, loans, and rules toggle
    int chooseRiskRoute(const Player& player, const RuleSet& rules);

//Purpose: decides whether CPU attends night school
//Relation: depends on difficulty, salary, cash, and rules toggle
    int chooseNightSchool(const Player& player, const RuleSet& rules);

//Purpose: decides retirement location (Millionaire Estates vs Countryside Acres)
//Relation: depends on difficulty and total worth
    int chooseRetirement(const Player& player);

//Purpose: selects career card from available options
//Relation: Easy = random, Normal = mostly best salary with some randomness, Hard = always best salary
    int chooseCareer(const Player& player, const std::vector<CareerCard>& choices);

//Purpose: simulates CPU performance in minigames (Pong, Battleship, Hangman, Memory, Minesweeper)
//Relation: difficulty rank sets score range and success threshold; summary text generated per minigame
    CpuMinigameResult playBlackTileMinigame(const Player& player, int minigameChoice);

//Purpose: decides whether CPU attempts sabotage
//Relation: depends on difficulty, cash, cooldown, and turn count
    bool shouldUseSabotage(const Player& player, int turnCounter);

//Purpose: selects target player for sabotage
//Relation: Hard = richest opponent, Easy = random opponent, Normal = richest with some randomness
    int chooseSabotageTarget(const Player& player,
                             const std::vector<Player>& players,
                             int selfIndex);

    //Purpose: decides sabotage type (MoneyLoss, MovementPenalty, DebtIncrease, StealCard, ItemDisable, PositionSwap, CareerPenalty, ForceMinigame)
    //Relation: depends on difficulty, target’s defenses, salaries, cash, and position
    SabotageType chooseSabotageType(const Player& player,
                                    const Player& target,
                                    int turnCounter);

private:
    RandomService& rng;

    int difficultyRank(const Player& player) const;
    bool chance(int percent);
};
