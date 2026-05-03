#pragma once

#include <string>

#include "rules.hpp"

struct GameSettings {
    std::string modeName = "Life Mode";
    bool customMode = false;

    int minJobSalary = 20000;
    int maxJobSalary = 120000;

    int collegeCost = 100000;

    int startingCash = 10000;
    int loanAmount = 50000;
    int loanPenalty = 60000;
    int maxLoans = 99;

    int paydayMultiplierPercent = 100;
    int taxMultiplierPercent = 100;
    int eventRewardMultiplierPercent = 100;
    int eventPenaltyMultiplierPercent = 100;

    int investmentMinReturnPercent = 100;
    int investmentMaxReturnPercent = 100;

    int babyCost = 0;
    int petCost = 0;
    int houseMinCost = 0;
    int houseMaxCost = 1000000;

    int minigameReward = 100;
    int minigamePenalty = 0;
    int minigameDifficultyModifier = 0;

    int sabotageUnlockTurn = 3;
    int sabotagePenaltyMultiplierPercent = 100;

    bool allowAutomaticLoans = true;
    bool allowSabotage = true;
    bool allowInvestments = true;
    bool allowPets = true;
    bool allowRandomEvents = true;
};

//Input: none
//Output: GameSettings tuned for easier play (lower costs, higher rewards)
//Purpose: provides preset configuration for Relax Mode
//Relation: used at game start when player selects Relax Mode.
GameSettings createRelaxModeSettings();

//Input: none
//Output: GameSettings tuned for balanced play
//Purpose: provides preset configuration for Life Mode (default)
//Relation: used at game start when player selects Life Mode.
GameSettings createLifeModeSettings();

//Input: none
//Output: GameSettings tuned for harder play (higher costs, penalties)
//Purpose: provides preset configuration for Hell Mode
//Relation: used at game start when player selects Hell Mode.
GameSettings createHellModeSettings();

//Input: reference to GameSettings
//Output: modifies settings if invalid values detected
//Purpose: ensures settings are within safe ranges (e.g., non-negative, logical bounds)
//Relation: called before applying settings to rules.
void validateGameSettings(GameSettings& settings);

//Input: GameSettings, RuleSet reference
//Output: modifies RuleSet according to settings
//Purpose: translates high-level settings into concrete rule values
//Relation: bridges configuration layer (GameSettings) with gameplay rules (RuleSet).
void applyGameSettingsToRules(const GameSettings& settings, RuleSet& rules);

//Input: settings reference, color flag
//Output: true if custom settings applied, false if cancelled
//Purpose: interactive menu for player to adjust settings
//Relation: allows player customization beyond presets.
bool showCustomSettingsMenu(GameSettings& settings, bool hasColor);

//Input: GameSettings
//Output: string summary of settings
//Purpose: generates human-readable overview of current configuration
//Relation: used in UI or logs to display chosen mode and parameters.
std::string gameSettingsSummary(const GameSettings& settings);
