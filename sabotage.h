#pragma once

#include <string>
#include <vector>

#include "bank.hpp"
#include "player.hpp"
#include "random_service.hpp"
#include "sabotage_card.h"

//Input: none
//Output: ActiveTrap structure
//Purpose: builds the features of default trap
//Relation: determine trap penalties
struct ActiveTrap {
    int tileId = 0;
    int ownerIndex = -1;
    SabotageType effectType = SabotageType::MoneyLoss;
    int strengthLevel = 1;
    bool armed = true;
};

//Input: none
//Output: SabotageResult
//Purpose: update sabotage summary
//Relation: determine sabotage outcome
struct SabotageResult {
    bool attempted = false;
    bool blocked = false;
    bool success = false;
    bool critical = false;
    int roll = 0;
    int amount = 0;
    std::string summary;
};

class SabotageManager {
//Input: parameters for each type of sabotage
//Output: result for each type of sabotage
//Purpose: declares each type of sabotage
//Relation: manages each type of sabotage
public:
    SabotageManager(Bank& bank, RandomService& random);

    SabotageResult applyDirectSabotage(const SabotageCard& card,
                                       Player& attacker,
                                       Player& target,
                                       std::vector<Player>& players,
                                       int attackerIndex,
                                       int targetIndex);
    SabotageResult applyDirectSabotage(const SabotageCard& card,
                                       Player& attacker,
                                       Player& target,
                                       std::vector<Player>& players,
                                       int attackerIndex,
                                       int targetIndex,
                                       int forcedRoll);
    SabotageResult triggerTrap(const ActiveTrap& trap, Player& target);
    SabotageResult triggerTrap(const ActiveTrap& trap, Player& target, int forcedRoll);
    SabotageResult resolveLawsuit(Player& attacker, Player& target);
    SabotageResult resolveLawsuit(Player& attacker, Player& target, int attackerRoll, int targetRoll);
    SabotageResult resolveForcedDuel(Player& attacker, Player& target);
    bool consumeShield(Player& target, std::string& message);
    int applyInsurance(Player& target, int amount, std::string& message);

//Input: random event percentages, success/critical flags
//Output: random event outcome, and penalty amounts
//Purpose: provides access to bank, and rng features
//Relation: used to resolve sabotages
private:
    Bank& bank;
    RandomService& rng;

    int rollOutcome(const SabotageCard& card, bool& success, bool& critical);
    int moneyAmount(int baseAmount, bool critical) const;
    bool rollChance(int percent);
};

// To add a future sabotage type:
// 1. Add a SabotageType value and card metadata in sabotage_card.cpp.
// 2. Add the effect implementation in SabotageManager::applyDirectSabotage.
// 3. Add any persistent state to Player or ActiveTrap, then save/load it.
