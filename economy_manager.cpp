#include "game.hpp"
#include "ui.h"
#include "ui_helpers.h"
#include "spins.hpp"
#include "tutorials.h"

#include <algorithm>
#include <sstream>
#include <string>

std::string trimCopy(const std::string& text);  // defined in game.cpp

//Input: Base string, PaymentResult object.
//Output: String with loan information appended if loans were taken.
//Purpose: Adds explanatory text when automatic loans are triggered.
//Relation: Used in many places (buying houses, risky route, night school, etc.) to clarify loan mechanics.
std::string appendLoanText(const std::string& base, const PaymentResult& payment) {
    if (payment.loansTaken <= 0) {
        return base;
    }

    std::ostringstream out;
    out << base << " The bank covered it with " << payment.loansTaken
        << " automatic loan" << (payment.loansTaken == 1 ? "" : "s") << ".";
    return out.str();
}

//Input: Career card.
//Output: String description of career.
//Purpose: Provides a readable description of a career card, falling back to defaults if none provided.
//Relation: Used in career selection UI (chooseCareer)
std::string careerDescriptionText(const CareerCard& card) {
    const std::string description = trimCopy(card.description);
    if (!description.empty() && description != card.title) {
        return description;
    }
    if (card.requiresDegree) {
        return "A degree-focused career with stronger long-term salary potential.";
    }
    return "A practical career path that starts earning steady pay right away.";
}

//Input: Player reference.
//Output: Adjusted salary after sabotage reductions.
//Purpose: Calculates salary considering penalties.
//Relation: Used in payday tile resolution and scoring.
int Game::effectiveSalary(const Player& player) const {
    if (player.salaryReductionTurns <= 0 || player.salaryReductionPercent <= 0) {
        return player.salary;
    }
    const int reduction = (player.salary * player.salaryReductionPercent) / 100;
    return std::max(0, player.salary - reduction);
}

//Input: None.
//Output: None.
//Purpose: Assigns investments to players if enabled.
//Relation: Called during game setup.
void Game::setupInvestments() {
    if (!rules.toggles.investmentEnabled) {
        return;
    }

    for (size_t i = 0; i < players.size(); ++i) {
        assignInvestment(players[i]);
    }
}

//Input: Player index, tile, action effect, reference to amount delta.
//Output: String summary of effect.
//Purpose: Applies an action card effect (gain/pay cash, per kid, salary bonus, marriage bonus, movement, duel minigame).
//Relation: Central handler for action cards. Calls resolveDuelMinigameAction, movePlayerByAction
std::string Game::applyActionEffect(int playerIndex,
                                    const Tile& tile,
                                    const ActionEffect& effect,
                                    int& amountDelta) {
    Player& player = players[playerIndex];
    amountDelta = 0;

    PaymentResult payment;
    payment.charged = 0;
    payment.loansTaken = 0;

    int amount = effect.amount;
    if (effect.useTileValue) {
        amount += tile.value * 2000;
    }

    switch (effect.kind) {
        case ACTION_NO_EFFECT:
            return "No payout this time.";
        case ACTION_GAIN_CASH:
            bank.credit(player, amount);
            amountDelta = amount;
            return "Collected $" + std::to_string(amount) + ".";
        case ACTION_PAY_CASH:
            payment = bank.charge(player, amount);
            maybeShowLoanTutorial(playerIndex, payment);
            amountDelta = -amount;
            return appendLoanText("Paid $" + std::to_string(amount) + ".", payment);
        case ACTION_GAIN_PER_KID:
            amount = player.kids * effect.amount;
            bank.credit(player, amount);
            amountDelta = amount;
            return "Collected $" + std::to_string(amount) + " for family bonuses.";
        case ACTION_PAY_PER_KID:
            amount = player.kids * effect.amount;
            payment = bank.charge(player, amount);
            maybeShowLoanTutorial(playerIndex, payment);
            amountDelta = -amount;
            return appendLoanText("Paid $" + std::to_string(amount) + " for family costs.", payment);
        case ACTION_GAIN_SALARY_BONUS:
            player.salary += effect.amount;
            bank.credit(player, effect.amount);
            amountDelta = effect.amount;
            return "Salary +$" + std::to_string(effect.amount) + " and immediate bonus paid.";
        case ACTION_BONUS_IF_MARRIED:
            if (player.married) {
                bank.credit(player, effect.amount);
                amountDelta = effect.amount;
                return "Marriage bonus paid $" + std::to_string(effect.amount) + ".";
            }
            return "No payout because you are not married yet.";
        case ACTION_MOVE_SPACES:
            return movePlayerByAction(playerIndex, effect.amount);
        case ACTION_DUEL_MINIGAME:
            return resolveDuelMinigameAction(playerIndex, amountDelta);
        default:
            return "No effect.";
    }
}

//Input: Player reference, boolean requiresDegree.
//Output: None.
//Purpose: Lets player (or CPU) choose a career card, sets job and salary.
//Relation: Called at career tiles, graduation, or night school. Uses careerDescriptionText, decks.drawCareerChoices
void Game::chooseCareer(Player& player, bool requiresDegree) {
    std::vector<CareerCard> choices = decks.drawCareerChoices(requiresDegree, 2);
    if (choices.empty()) {
        showInfoPopup("Career Deck", "No career cards are available.");
        return;
    }
    const int playerIndex = findPlayerIndex(player);
    if (!isCpuPlayer(playerIndex)) {
        maybeShowFirstTimeTutorial(TutorialTopic::Job);
    }

    std::vector<std::string> lines;
    for (size_t i = 0; i < choices.size(); ++i) {
        lines.push_back("- " + choices[i].title + ": " +
                        careerDescriptionText(choices[i]) +
                        " Salary $" + std::to_string(choices[i].salary) +
                        (choices[i].requiresDegree ? ". Degree required." : ". No degree required."));
    }

    int choice = 0;
    if (choices.size() > 1) {
        if (isCpuPlayer(playerIndex)) {
            choice = cpu.chooseCareer(player, choices);
            showCpuThinking(playerIndex,
                            "Career choice: " + choices[static_cast<std::size_t>(choice)].title +
                            " ($" + std::to_string(choices[static_cast<std::size_t>(choice)].salary) + ")");
        } else {
            choice = showRequiredBranchPopup(requiresDegree ? "Choose a college career" : "Choose a career", lines, 'A', 'B');
        }
    }

    decks.resolveCareerChoices(requiresDegree, choices, choice);

    player.job = choices[choice].title;
    player.salary = choices[choice].salary;
    if (requiresDegree) {
        player.collegeGraduate = true;
    }

    addHistory(player.name + " became " + player.job);
    showInfoPopup(player.name + " became a " + player.job,
                  careerDescriptionText(choices[static_cast<std::size_t>(choice)]) +
                  " Salary: $" + std::to_string(player.salary) +
                  (choices[static_cast<std::size_t>(choice)].requiresDegree
                       ? ". Degree required."
                       : ". No degree required."));
}

//Input: Player reference.
//Output: None.
//Purpose: Resolves family vs life path choice.
//Relation: Called at Family split tile. Updates player state (hasFamilyPath, familyBabyEventsRemaining)
void Game::resolveFamilyStop(Player& player) {
    if (!rules.toggles.familyPathEnabled) {
        player.familyChoice = 1;
        addHistory(player.name + " stays on the life path");
        showInfoPopup("Family STOP", "Family path is disabled. Staying on the life path.");
        return;
    }

    const int playerIndex = findPlayerIndex(player);
    int choice = 0;
    if (isCpuPlayer(playerIndex)) {
        choice = cpu.chooseFamilyRoute(player, rules);
        showCpuThinking(playerIndex,
                        choice == 0 ? "CPU chose Family path." : "CPU chose Life path.");
    } else {
        choice = showRequiredBranchPopup(
            "Family or Life path?",
            std::vector<std::string>{
                "- FAMILY PATH: Start a family, have children, build a home",
                "- LIFE PATH: Focus on careers, safe/risky route"
            },
            'A',
            'B');
    }
    
    player.familyChoice = choice;

    if (choice == 0) {
        // Family path - enable baby events
        player.hasFamilyPath = true;
        player.familyBabyEventsRemaining = 3;
        addHistory(player.name + " chose the Family path");
        showInfoPopup("Family Path", "Children may arrive in future events!");
    } else {
        // Work path - normal continuation
        player.hasFamilyPath = false;
        addHistory(player.name + " chose the Work path");
        showInfoPopup("Work Path", "Focusing on career advancement.");
    }
}

//Input: Player reference.
//Output: None.
//Purpose: Resolves family baby event: spins for babies, charges cost, updates player’s kids.
//Relation: Called at Family tiles when events remain. Uses babiesFromSpin, bank.charge
void Game::triggerBabyEvent(Player& player) {
    if (!player.hasFamilyPath) return;
    if (player.familyBabyEventsRemaining <= 0) return;
    
    const int playerIndex = findPlayerIndex(player);
    player.familyBabyEventsRemaining--;
    
    // Show narrative in the message window
    renderGame(playerIndex, 
               player.name + " receives happy news!", 
               "The family is growing... Press SPACE to see how many babies!");
    
    // Spin for number of babies
    int spin = rollSpinner("Baby News", "Hold SPACE to see how many babies you'll have");
    int babies = babiesFromSpin(spin);
    
    if (babies == 0) {
        showInfoPopup("Family Event", "No babies this time. Maybe next time!");
        addHistory(player.name + " hoped for children but none arrived yet.");
        return;
    }
    
    // Calculate cost: $10,000 per baby
    int cost = babies * 10000;
    PaymentResult payment = bank.charge(player, cost);
    maybeShowLoanTutorial(playerIndex, payment);
    
    // Add babies to player
    player.kids += babies;
    
    // Show result in info popup
    std::string result = babiesLabel(babies) + " arrived! Cost: $" + std::to_string(cost);
    if (payment.loansTaken > 0) {
        result += " (Auto-loan: " + std::to_string(payment.loansTaken) + ")";
    }
    
    addHistory(player.name + " had " + babiesLabel(babies) + " and paid $" + std::to_string(cost));
    showInfoPopup(" NEW ARRIVAL! ", result);
    
    // Update the game display
    renderGame(playerIndex, 
               player.name + " welcomes " + babiesLabel(babies), 
               "Total children: " + std::to_string(player.kids));
    napms(1500);
}

//Input: Player reference.
//Output: None.
//Purpose: Resolves Night School choice: pay $100,000 to change career or keep current.
//Relation: Called at Night School tile. Uses chooseCareer
void Game::resolveNightSchool(Player& player) {
    if (!rules.toggles.nightSchoolEnabled) {
        addHistory(player.name + " passed Night School");
        showInfoPopup("Night School", "Night School is disabled in this mode.");
        return;
    }
    if (player.usedNightSchool) {
        showInfoPopup("Night School", "You already used Night School.");
        return;
    }

    const int playerIndex = findPlayerIndex(player);
    int choice = 0;
    if (isCpuPlayer(playerIndex)) {
        choice = cpu.chooseNightSchool(player, rules);
        showCpuThinking(playerIndex,
                        choice == 0 ? "CPU chose Night School." : "CPU kept current career.");
    } else {
        choice = showRequiredBranchPopup(
            "Night School?",
            std::vector<std::string>{
                "- Pay $100000 to draw a new college career",
                "- Keep your current career"
            },
            'A',
            'B');
    }
    if (choice == 0) {
        PaymentResult payment = bank.charge(player, 100000);
        maybeShowLoanTutorial(playerIndex, payment);
        player.usedNightSchool = true;
        addHistory(appendLoanText(player.name + " paid $100000 for Night School", payment));
        chooseCareer(player, true);
    } else {
        addHistory(player.name + " skipped Night School");
        showInfoPopup("Night School", "Current career kept.");
    }
}

//Input: Player reference.
//Output: None.
//Purpose: Resolves marriage event: sets married flag, spins for wedding gifts, credits bank.
//Relation: Called at Marriage tile. May award pet card
void Game::resolveMarriageStop(Player& player) {
    if (!player.married) {
        player.married = true;
    }
    const int playerIndex = findPlayerIndex(player);
    if (!isCpuPlayer(playerIndex)) {
        maybeShowFirstTimeTutorial(TutorialTopic::Marriage);
    }
    int spin = rollSpinner("Marriage Gifts", "Hold SPACE to spin wedding gifts");
    int gift = marriageGiftFromSpin(spin);
    bank.credit(player, gift);
    addHistory(player.name + " married and received $" + std::to_string(gift));
    maybeAwardPetCard(player, "Family edition bonus: a pet joined the family.");
    showInfoPopup("Get Married", "Gift spin paid $" + std::to_string(gift) + ".");
}

//Input: None.
//Output: None.
//Purpose: Final scoring step: sells houses, calculates scores, shows breakdown.
//Relation: Called at endgame before winner determination. Uses houseSaleValueFromSpin, updates history/UI.
void Game::finalizeScoring() {
    for (size_t i = 0; i < players.size(); ++i) {
        Player& player = players[i];
        const bool previousAutoAdvance = autoAdvanceUi;
        autoAdvanceUi = isCpuPlayer(static_cast<int>(i));
        if (player.hasHouse) {
            if (rules.toggles.houseSaleSpinEnabled) {
                int spin = rollSpinner(player.name + " house sale", "Spin to sell your house");
                player.finalHouseSaleValue = houseSaleValueFromSpin(player.houseValue, spin);
            } else {
                player.finalHouseSaleValue = player.houseValue;
            }
            addHistory(player.name + " sold " + player.houseName + " for $" + std::to_string(player.finalHouseSaleValue));
        }

        int actionScore = static_cast<int>(player.actionCards.size()) * 100000;
        int petScore = static_cast<int>(player.petCards.size()) * 100000;
        int babyScore = player.kids * 50000;
        int loanPenalty = bank.totalLoanDebt(player);
        int houseScore = player.finalHouseSaleValue;

        std::ostringstream line1;
        std::ostringstream line2;
        line1 << player.name << ": cash $" << player.cash
              << " + house $" << houseScore
              << " + actions $" << actionScore;
        line2 << "pets $" << petScore
              << " + babies $" << babyScore
              << " + retire $" << player.retirementBonus
              << " - loans $" << loanPenalty;
        showInfoPopup(line1.str(), line2.str());
        autoAdvanceUi = previousAutoAdvance;
    }
}
//Input: Player reference.
//Output: Integer final worth.
//Purpose: Calculates total worth including cash, house, actions, pets, babies, retirement bonus, minus loans.
//Relation: Used in endgame ranking and winner determination.
int Game::calculateFinalWorth(const Player& player) const {
    int worth = player.cash;
    worth += player.finalHouseSaleValue > 0 ? player.finalHouseSaleValue : player.houseValue;
    worth += static_cast<int>(player.actionCards.size()) * 100000;
    worth += static_cast<int>(player.petCards.size()) * 100000;
    worth += player.kids * 50000;
    worth += player.retirementBonus;
    worth -= bank.totalLoanDebt(player);
    return worth;
}
