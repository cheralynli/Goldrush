#pragma once

#include "player.hpp"
#include "rules.hpp"

struct PaymentResult {
    int charged;
    int loansTaken;
};

class Bank {
public:
    explicit Bank(const RuleSet& rules);

    void configure(const RuleSet& rules);
    void credit(Player& player, int amount) const;
    PaymentResult charge(Player& player, int amount) const;
    int issueLoan(Player& player, int amount) const;
    bool repayOneLoan(Player& player) const;
    int totalLoanDebt(const Player& player) const;

private:
    RuleSet ruleset;
};
