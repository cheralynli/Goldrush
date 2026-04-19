#include "bank.hpp"

Bank::Bank(const RuleSet& rules)
    : ruleset(rules) {
}

void Bank::configure(const RuleSet& rules) {
    ruleset = rules;
}

void Bank::credit(Player& player, int amount) const {
    if (amount <= 0) {
        return;
    }
    player.cash += amount;
}

PaymentResult Bank::charge(Player& player, int amount) const {
    PaymentResult result;
    result.charged = amount;
    result.loansTaken = 0;

    if (amount <= 0) {
        return result;
    }

    player.cash -= amount;
    while (player.cash < 0) {
        player.cash += ruleset.loanUnit;
        ++player.loans;
        ++result.loansTaken;
    }
    return result;
}

int Bank::issueLoan(Player& player, int amount) const {
    if (amount <= 0) {
        return 0;
    }

    int loansIssued = amount / ruleset.loanUnit;
    if (amount % ruleset.loanUnit != 0) {
        ++loansIssued;
    }

    player.loans += loansIssued;
    player.cash += loansIssued * ruleset.loanUnit;
    return loansIssued;
}

bool Bank::repayOneLoan(Player& player) const {
    if (player.loans <= 0) {
        return false;
    }
    if (player.cash < ruleset.loanRepaymentCost) {
        return false;
    }
    player.cash -= ruleset.loanRepaymentCost;
    --player.loans;
    return true;
}

int Bank::totalLoanDebt(const Player& player) const {
    return player.loans * ruleset.loanRepaymentCost;
}
