#include "rules.hpp"

namespace {
//Input:name → string label for the edition (e.g., "Normal Mode", "Custom Mode")
//Output:RuleSet object fully initialized with default toggles, components, and loan/investment parameters
//Purpose:Provides a reusable baseline configuration of game rules with all features enabled and standard component counts
//Relation:Called by makeNormalRules and makeCustomRules to generate specific editions, centralizes rule initialization so changes only need to be made in one place.
RuleSet makeBaseRules(const std::string& name) {
    RuleSet rules;
    rules.editionName = name;
    rules.toggles.petsEnabled = true;
    rules.toggles.investmentEnabled = true;
    rules.toggles.spinToWinEnabled = true;
    rules.toggles.electronicBankingEnabled = true;
    rules.toggles.familyPathEnabled = true;
    rules.toggles.riskyRouteEnabled = true;
    rules.toggles.nightSchoolEnabled = true;
    rules.toggles.tutorialEnabled = true;
    rules.toggles.houseSaleSpinEnabled = true;
    rules.toggles.retirementBonusesEnabled = true;
    rules.components.actionCards = 55;
    rules.components.collegeCareerCards = 10;
    rules.components.careerCards = 10;
    rules.components.houseCards = 11;
    rules.components.investCards = 4;
    rules.components.petCards = 12;
    rules.components.spinToWinTokens = 5;
    rules.loanUnit = 50000;
    rules.loanRepaymentCost = 60000;
    rules.maxLoans = 99;
    rules.automaticLoansEnabled = true;
    rules.investmentMatchPayout = 25000;
    rules.spinToWinPrize = 25000;
    return rules;
}
}

//Input: none
//Output: RuleSet configured with "Normal Mode" edition name
//Purpose: returns a standard ruleset for the default game mode
//Relation: wraps makeBaseRules("Normal Mode").
RuleSet makeNormalRules() {
    return makeBaseRules("Normal Mode");
}

//Input: none
//Output: RuleSet configured with "Custom Mode" edition name
//Purpose: returns a ruleset for custom gameplay mode
//Relation: wraps makeBaseRules("Custom Mode").
RuleSet makeCustomRules() {
    return makeBaseRules("Custom Mode");
}
