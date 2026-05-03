#pragma once

#include "player.hpp"
#include "rules.hpp"

struct PaymentResult {
    int charged;
    int loansTaken;
};

class Bank {
public:
    //Input: RuleSet (rules)
    //Output: none
    //Purpose: initializes Bank with given ruleset
    //Relation: ruleset affects all function below (loan,credit,charge behavior)
    explicit Bank(const RuleSet& rules);

    //Input: RuleSet (rules)
    //Output: none
    //Purpose: updates the Bank's ruleset
    //Relation: changes how credit, charge, issueLoan, repayOneLoan work
    void configure(const RuleSet& rules);
    //Input: Player& (player), int (amount)
    //Output: none
    //Purpose: adds cash to player if amount > 0
    //Relation: complements charge() (adds vs subtracts cash)
    void credit(Player& player, int amount) const;
    //Input: Player& (player), int (amount)
    //Output: PaymentResult (charged amount + loans taken)
    //Purpose: subtracts cash; may issue automatic loans if cash < 0
    //Relation: interacts with issueLoan func, and loans taken here must later be repaid via repayOneLoan func
    PaymentResult charge(Player& player, int amount) const;
    //Input: Player& (player), int (amount requested)
    //Output: int (number of loans issued)
    //Purpose: manually issues loans in multiples of loanUnit
    //Relation: similar to charge funv but explicit
    int issueLoan(Player& player, int amount) const;
    //Input: Player& (player)
    //Output: bool (true if repayment succeeded)
    //Purpose: repays one loan if player has loans and enough cash
    //Relation: reduces loans created by charge func or issueLoan func, directly affects totalLoanDebt func
    bool repayOneLoan(Player& player) const;
    //Input: const Player& (player)
    //Output: int(total debt)
    //Purpose: calculates total debt = loans * repayment cost
    //Relation: depends on loans created by issueLoan func/charge func, reduced by repayOneLoanfunc
    int totalLoanDebt(const Player& player) const;

private:
    RuleSet ruleset;
};
