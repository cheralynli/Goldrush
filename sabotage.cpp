#include "sabotage.h"

#include <algorithm>

namespace {
//Input: Player player, rng generator
//Output: random integer (Human:40-85, cpuEasy:20-55, cpuNormal:45-75, cpuHard:65-95)
//Purpose: generates random score for Forced Duel minigame
//Relation: compare duel score to decide winner of Forced Duel
int cpuDuelScore(const Player& player, RandomService& rng) {
    if (player.type != PlayerType::CPU) {
        return rng.uniformInt(40, 85);
    }

    switch (player.cpuDifficulty) {
        case CpuDifficulty::Easy:
            return rng.uniformInt(20, 55);
        case CpuDifficulty::Hard:
            return rng.uniformInt(65, 95);
        case CpuDifficulty::Normal:
        default:
            return rng.uniformInt(45, 75);
    }
}

//Input: Player player
//Output: profile type of player
//Purpose: determine type of player to display on interface
//Relation: used when showing Forced Duel matchups
std::string duelProfileText(const Player& player) {
    if (player.type != PlayerType::CPU) {
        return "Human controlled";
    }
    return "CPU difficulty: " + cpuDifficultyLabel(player.cpuDifficulty);
}
}  // namespace

namespace {
//Input: SabotageCard, int FixedRoll, bool success flag, bool critical flag
//Output: FixedRoll value
//Purpose: toggles success/critical flags and returns FixedRoll output
//Relation: returns a fixed roll value for debugging traptile/lawsuit
int rollOutcomeFromValue(const SabotageCard& card, int roll, bool& success, bool& critical) {
    critical = false;
    if (!card.requiresDiceRoll) { //trap tile
        success = roll <= card.successChance;
        return 0;
    }

    if (roll <= 3) { //lawsuit
        success = false;
        return roll;
    }
    success = true;
    critical = roll >= 8;
    return roll;
}
}

//Input: bankRef, random
//Output: none
//Purpose: binds SabotageManager to external Bank and RandomService dependencies
//Relation: called when creating a SabotageManager instance
SabotageManager::SabotageManager(Bank& bankRef, RandomService& random)
    : bank(bankRef),
      rng(random) {
}

//Input: int percentage chance of TRUE
//Output: bool value returns true with percent chance
//Purpose: determines if a random event suceeds based on a given percentage chance
//Relation: gets called if !requiresDiceRoll 
bool SabotageManager::rollChance(int percent) {
    if (percent <= 0) {
        return false;
    }
    if (percent >= 100) {
        return true;
    }
    return rng.uniformInt(1, 100) <= percent;
}

//Input: SabotageCard, bool success flag, bool critical flag
//Output: int roll
//Purpose: toggles success/critical flags and returns roll output
//Relation: determines success/critical result for traptile/lawsuit
int SabotageManager::rollOutcome(const SabotageCard& card, bool& success, bool& critical) {
    critical = false;
    if (!card.requiresDiceRoll) { // trap tile
        success = rollChance(card.successChance);
        return 0;
    }

    const int roll = rng.roll10(); // lawsuit
    if (roll <= 3) {
        success = false;
        return roll;
    }
    success = true;
    critical = roll >= 8;
    return roll;
}

//Input: int baseAmount, bool critical flag
//Output: int moneyAmount (baseAmount*2 if critical flag is raised)
//Purpose: double sabotage effect if critical flag is raised
//Relation: increases sabotage effects depending on chance
int SabotageManager::moneyAmount(int baseAmount, bool critical) const {
    return critical ? baseAmount * 2 : baseAmount;
}

//Input: Player target
//Output: bool consumeShield
//Purpose: checks if player has a shield available
//Relation: for use against tile traps
bool SabotageManager::consumeShield(Player& target, std::string& message) {
    if (target.itemDisableTurns > 0) {
        return false;
    }
    if (target.shieldCards <= 0) {
        return false;
    }
    --target.shieldCards;
    message = target.name + "'s Shield Card blocked the sabotage.";
    return true;
}

//Input: Player target, int charged amount 
//Output: Reduced charged amount
//Purpose: decrease amount owed if player has insurance, decrement target.insuranceUses.
//Relation: for use against lawsuit, tile traps, property damage 
int SabotageManager::applyInsurance(Player& target, int amount, std::string& message) {
    if (target.itemDisableTurns > 0 || target.insuranceUses <= 0 || amount <= 0) {
        return amount;
    }
    --target.insuranceUses;
    const int reduced = amount / 2;
    message = "Insurance reduced damage from $" + std::to_string(amount) +
              " to $" + std::to_string(reduced) + ".";
    return reduced;
}

//Input: Player attacker, Player target
//Output: resolveLawsuit()
//Purpose: simplify resolveLawsuit not needing to pass rng.roll10()
//Relation: convenience wrapper for resolveLawsuit()
SabotageResult SabotageManager::resolveLawsuit(Player& attacker, Player& target) {
    return resolveLawsuit(attacker, target, rng.roll10(), rng.roll10());
}

//Input: Player attacker, Player target, attackerRoll rng.roll10(), targetRoll rng.roll10()
//Output: lawsuit result
//Purpose: calculates amount owed in lawsuit sabotage and updates summary
//Relation: resolves lawsuit sabotage
SabotageResult SabotageManager::resolveLawsuit(Player& attacker, Player& target, int attackerRoll, int targetRoll) {
    SabotageResult result;
    result.attempted = true;
    result.roll = attackerRoll;

    if (attackerRoll > targetRoll) {
        int amount = 50000 + (attackerRoll - targetRoll) * 5000;
        std::string insuranceText;
        amount = applyInsurance(target, amount, insuranceText);
        PaymentResult payment = bank.charge(target, amount);
        bank.credit(attacker, amount);
        result.success = true;
        result.amount = amount;
        result.summary = attacker.name + " won the lawsuit roll " +
                         std::to_string(attackerRoll) + "-" + std::to_string(targetRoll) +
                         ". " + target.name + " paid $" + std::to_string(amount) + ".";
        if (!insuranceText.empty()) {
            result.summary += " " + insuranceText;
        }
        if (payment.loansTaken > 0) {
            result.summary += " Automatic loans: " + std::to_string(payment.loansTaken) + ".";
        }
        return result;
    }

    const int amount = 25000;
    PaymentResult payment = bank.charge(attacker, amount);
    bank.credit(target, amount);
    result.success = false;
    result.amount = amount;
    result.summary = attacker.name + " lost the lawsuit roll " +
                     std::to_string(attackerRoll) + "-" + std::to_string(targetRoll) +
                     " and paid $" + std::to_string(amount) + ".";
    if (payment.loansTaken > 0) {
        result.summary += " Automatic loans: " + std::to_string(payment.loansTaken) + ".";
    }
    return result;
}

//Input: Player attacker, Player target
//Output: Force Duel result
//Purpose: calculates amount owed in Forced Duel sabotage and updates summary
//Relation: resolves Forced Duel sabotage
SabotageResult SabotageManager::resolveForcedDuel(Player& attacker, Player& target) {
    SabotageResult result;
    result.attempted = true;
    const int attackerScore = cpuDuelScore(attacker, rng);
    const int targetScore = cpuDuelScore(target, rng);
    result.roll = attackerScore;
    const int pot = 40000;

    if (attackerScore > targetScore) {
        PaymentResult payment = bank.charge(target, pot);
        bank.credit(attacker, pot);
        result.success = true;
        result.amount = pot;
        result.summary = attacker.name + " won the duel minigame " +
                         std::to_string(attackerScore) + "-" + std::to_string(targetScore) +
                         " and took $" + std::to_string(pot) + ". " +
                         duelProfileText(attacker) + " vs " + duelProfileText(target) + ".";
        if (payment.loansTaken > 0) {
            result.summary += " Automatic loans: " + std::to_string(payment.loansTaken) + ".";
        }
    } else {
        PaymentResult payment = bank.charge(attacker, pot / 2);
        bank.credit(target, pot / 2);
        result.success = false;
        result.amount = pot / 2;
        result.summary = target.name + " defended the duel minigame " +
                         std::to_string(targetScore) + "-" + std::to_string(attackerScore) +
                         " and collected $" + std::to_string(pot / 2) + ". " +
                         duelProfileText(attacker) + " vs " + duelProfileText(target) + ".";
        if (payment.loansTaken > 0) {
            result.summary += " Automatic loans: " + std::to_string(payment.loansTaken) + ".";
        }
    }
    return result;
}

//Input: SabotageCard, Player attacker, Player target, vector Players, int attackerIndex, int targetIndex
//Output: applyDirectSabotage()
//Purpose: simplify applyDirectSabotage without passing a forcedRoll
//Relation: convenience wrapper for applyDirectSabotage()
SabotageResult SabotageManager::applyDirectSabotage(const SabotageCard& card,
                                                    Player& attacker,
                                                    Player& target,
                                                    std::vector<Player>& players,
                                                    int attackerIndex,
                                                    int targetIndex) {
    return applyDirectSabotage(card, attacker, target, players, attackerIndex, targetIndex, -1);
}

//Input: SabotageCard, Player attacker, Player target, vector Players, int attackerIndex, int targetIndex, int forcedRoll
//Output: Sabotage result for other sabotages
//Purpose: calculates amount owed for each sabotage and updates summary
//Relation: resolves other sabotages
SabotageResult SabotageManager::applyDirectSabotage(const SabotageCard& card,
                                                    Player& attacker,
                                                    Player& target,
                                                    std::vector<Player>& players,
                                                    int attackerIndex,
                                                    int targetIndex,
                                                    int forcedRoll) {
    SabotageResult result;
    result.attempted = true;

    if (targetIndex < 0 || targetIndex >= static_cast<int>(players.size()) || attackerIndex == targetIndex) {
        result.summary = "Invalid sabotage target.";
        return result;
    }

    std::string shieldText;
    if (consumeShield(target, shieldText)) {
        result.blocked = true;
        result.summary = shieldText;
        return result;
    }

    if (card.type == SabotageType::MoneyLoss && card.name == "Lawsuit") {
        if (forcedRoll > 0) {
            return resolveLawsuit(attacker, target, forcedRoll, rng.roll10());
        }
        return resolveLawsuit(attacker, target);
    }

    bool success = false;
    bool critical = false;
    result.roll = forcedRoll > 0
        ? rollOutcomeFromValue(card, forcedRoll, success, critical)
        : rollOutcome(card, success, critical);
    result.critical = critical;
    if (!success) {
        result.summary = card.name + " failed";
        if (result.roll > 0) {
            result.summary += " on roll " + std::to_string(result.roll);
        }
        result.summary += ".";
        return result;
    }

    switch (card.type) {
        case SabotageType::MoneyLoss: {
            int amount = moneyAmount(30000, critical);
            std::string insuranceText;
            amount = applyInsurance(target, amount, insuranceText);
            PaymentResult payment = bank.charge(target, amount);
            result.success = true;
            result.amount = amount;
            result.summary = target.name + " lost $" + std::to_string(amount) + ".";
            if (!insuranceText.empty()) {
                result.summary += " " + insuranceText;
            }
            if (payment.loansTaken > 0) {
                result.summary += " Automatic loans: " + std::to_string(payment.loansTaken) + ".";
            }
            break;
        }
        case SabotageType::MovementPenalty:
            target.movementPenaltyTurns = 1;
            target.movementPenaltyPercent = critical ? 100 : 50;
            result.success = true;
            result.summary = target.name + "'s next movement is " +
                             std::string(critical ? "cancelled." : "cut in half.");
            break;
        case SabotageType::SkipTurn:
            target.skipNextTurn = true;
            result.success = true;
            result.summary = target.name + " will skip their next turn.";
            break;
        case SabotageType::StealCard:
            if (target.actionCards.empty()) {
                result.summary = target.name + " has no action cards to steal.";
                return result;
            } else {
                const int cardIndex = rng.uniformInt(0, static_cast<int>(target.actionCards.size()) - 1);
                const std::string stolen = target.actionCards[static_cast<std::size_t>(cardIndex)];
                target.actionCards.erase(target.actionCards.begin() + cardIndex);
                attacker.actionCards.push_back(stolen);
                result.success = true;
                result.summary = attacker.name + " stole action card: " + stolen + ".";
            }
            break;
        case SabotageType::ForceMinigame:
            return resolveForcedDuel(attacker, target);
        case SabotageType::CareerPenalty:
            target.salaryReductionTurns = critical ? 4 : 3;
            target.salaryReductionPercent = critical ? 35 : 25;
            result.success = true;
            result.summary = target.name + "'s salary is reduced by " +
                             std::to_string(target.salaryReductionPercent) + "% for " +
                             std::to_string(target.salaryReductionTurns) + " turns.";
            break;
        case SabotageType::PropertyDamage: {
            int amount = moneyAmount(25000, critical);
            std::string insuranceText;
            amount = applyInsurance(target, amount, insuranceText);
            PaymentResult payment = bank.charge(target, amount);
            result.success = true;
            result.amount = amount;
            result.summary = target.name + " paid $" + std::to_string(amount) + " for property damage.";
            if (!insuranceText.empty()) {
                result.summary += " " + insuranceText;
            }
            if (payment.loansTaken > 0) {
                result.summary += " Automatic loans: " + std::to_string(payment.loansTaken) + ".";
            }
            break;
        }
        case SabotageType::DebtIncrease: {
            const int debtAmount = moneyAmount(60000, critical);
            const int loans = bank.issueLoan(target, debtAmount);
            target.sabotageDebt += loans * 60000;
            result.success = true;
            result.amount = loans * 60000;
            result.summary = target.name + " was forced into " +
                             std::to_string(loans) + " loan(s).";
            break;
        }
        case SabotageType::PositionSwap:
            std::swap(attacker.tile, target.tile);
            attacker.sabotageCooldown = critical ? 3 : 4;
            result.success = true;
            result.summary = attacker.name + " swapped positions with " + target.name +
                             " and is on sabotage cooldown.";
            break;
        case SabotageType::ItemDisable:
            target.itemDisableTurns = critical ? 3 : 2;
            result.success = true;
            result.summary = target.name + "'s shields and insurance are disabled for " +
                             std::to_string(target.itemDisableTurns) + " turns.";
            break;
        default:
            result.summary = "No effect.";
            break;
    }

    return result;
}

//Input: ActiveTrap trap, Player target
//Output: triggerTrap()
//Purpose: simplify triggerTrap without passing a forcedRoll
//Relation: convenience wrapper for triggerTrap()
SabotageResult SabotageManager::triggerTrap(const ActiveTrap& trap, Player& target) {
    return triggerTrap(trap, target, -1);
}

//Input: ActiveTrap trap, Player target, int forcedRoll
//Output: Trap triggered results
//Purpose: checks for shield, calculates trap penalty, updates summary
//Relation: resolves landing on a trap tile
SabotageResult SabotageManager::triggerTrap(const ActiveTrap& trap, Player& target, int forcedRoll) {
    SabotageResult result;
    result.attempted = true;

    std::string shieldText;
    if (consumeShield(target, shieldText)) {
        result.blocked = true;
        result.summary = shieldText;
        return result;
    }

    const int roll = forcedRoll > 0 ? forcedRoll : rng.roll10();
    result.roll = roll;
    result.critical = roll >= 8;
    if (roll <= 3) {
        result.summary = target.name + " dodged the trap on roll " + std::to_string(roll) + ".";
        return result;
    }

    result.success = true;
    switch (trap.effectType) {
        case SabotageType::MoneyLoss: {
            int amount = moneyAmount(15000 * trap.strengthLevel, result.critical);
            std::string insuranceText;
            amount = applyInsurance(target, amount, insuranceText);
            PaymentResult payment = bank.charge(target, amount);
            result.amount = amount;
            result.summary = target.name + " triggered a money trap and lost $" +
                             std::to_string(amount) + ".";
            if (!insuranceText.empty()) {
                result.summary += " " + insuranceText;
            }
            if (payment.loansTaken > 0) {
                result.summary += " Automatic loans: " + std::to_string(payment.loansTaken) + ".";
            }
            break;
        }
        case SabotageType::MovementPenalty:
            result.summary = target.name + " triggered a movement trap.";
            break;
        case SabotageType::SkipTurn:
            target.skipNextTurn = true;
            result.summary = target.name + " triggered a skip-turn trap.";
            break;
        case SabotageType::StealCard:
            if (!target.actionCards.empty()) {
                const int cardIndex = rng.uniformInt(0, static_cast<int>(target.actionCards.size()) - 1);
                const std::string lost = target.actionCards[static_cast<std::size_t>(cardIndex)];
                target.actionCards.erase(target.actionCards.begin() + cardIndex);
                result.summary = target.name + " lost action card: " + lost + ".";
            } else {
                result.summary = target.name + " triggered a card trap but had no cards.";
            }
            break;
        case SabotageType::ForceMinigame:
            result.summary = target.name + " triggered a forced minigame trap.";
            break;
        default:
            result.summary = target.name + " triggered a trap.";
            break;
    }

    return result;
}
