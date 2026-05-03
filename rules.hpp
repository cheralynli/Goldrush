#pragma once

#include <string>

//Input: none (fields set when constructing a RuleSet)
//Output: boolean flags for optional game features
//Purpose: defines which gameplay mechanics are enabled (pets, investments, spin-to-win, banking, family path, risky route, night school, tutorial, house sale spin, retirement bonuses)
//Relation: embedded inside RuleSet to control feature availability across the game.
struct RuleToggles {
    bool petsEnabled;
    bool investmentEnabled;
    bool spinToWinEnabled;
    bool electronicBankingEnabled;
    bool familyPathEnabled;
    bool riskyRouteEnabled;
    bool nightSchoolEnabled;
    bool tutorialEnabled;
    bool houseSaleSpinEnabled;
    bool retirementBonusesEnabled;
};

//Input: none (fields set when constructing a RuleSet)
//Output: integer counts of card/token components
//Purpose: specifies how many of each deck or token type exist in the game (action cards, career cards, house cards, etc.)
//Relation: embedded inside RuleSet to define the physical/card resources available during gameplay.
struct ComponentSet {
    int actionCards;
    int collegeCareerCards;
    int careerCards;
    int houseCards;
    int investCards;
    int petCards;
    int spinToWinTokens;
};

//Input: edition name, toggles, components, loan parameters, investment payout, spin-to-win prize
//Output: complete configuration object representing one edition’s rules
//Purpose: central container for all rule-related settings (features, components, loans, payouts)
//Relation: Constructed by makeNormalRules and makeCustomRules, passed into systems like Bank (loan handling), Cards (deck sizes), and Minigames (toggle checks),provides a single source of truth for gameplay configuration.
struct RuleSet {
    std::string editionName;
    RuleToggles toggles;
    ComponentSet components;
    int loanUnit;
    int loanRepaymentCost;
    int maxLoans;
    bool automaticLoansEnabled;
    int investmentMatchPayout;
    int spinToWinPrize;
};
//Input: none
//Output: RuleSet configured with "Normal Mode" edition name
//Purpose: provides the default ruleset for standard gameplay
//Relation: wraps makeBaseRules("Normal Mode") in the implementation file; used during normal game setup.
RuleSet makeNormalRules();
//Input: none
//Output: RuleSet configured with "Custom Mode" edition name
//Purpose: provides a ruleset for custom gameplay mode
//Relation: wraps makeBaseRules("Custom Mode") in the implementation file; used when players want a variant edition.
RuleSet makeCustomRules();
