#pragma once

#include <ncurses.h>

#include <string>
#include <vector>

#include "board.hpp"
#include "rules.hpp"

//Input: tutorial flags
//Output: none
//Purpose: declare tutorial flags to be defaulted false
//Relation: initialise all flags to be false until displayed for the first time
struct TutorialFlags {
    bool automaticLoan = false;
    bool manualLoan = false;
    bool investment = false;
    bool job = false;
    bool baby = false;
    bool pet = false;
    bool marriage = false;
    bool house = false;
    bool insurance = false;
    bool shield = false;
    bool actionCard = false;
    bool minigame = false;
    bool sabotage = false;
    bool endgameScoring = false;
};

//Input: none
//Output: enumerated values representing tile types
//Purpose: defines all possible tutorial topics
//Relation: used by tutorialLines, tutorialTitle, tutorialFlagForTopic
enum class TutorialTopic {
    AutomaticLoan,
    ManualLoan,
    Investment,
    Job,
    Baby,
    Pet,
    Marriage,
    House,
    Insurance,
    Shield,
    ActionCard,
    Minigame,
    Sabotage,
    EndgameScoring
};

//Inputs: TutorialFlags flags, TutorialTopic topic
//Output: string tutorialTitle and tutorialLines, bool tutorialFlagForTopic
//Purpose: declare tutorial labels and flags
//Relation: manage tutorial UI state
bool& tutorialFlagForTopic(TutorialFlags& flags, TutorialTopic topic);
std::string tutorialTitle(TutorialTopic topic);
std::vector<std::string> tutorialLines(TutorialTopic topic);

//Inputs: by function
//Output: none
//Purpose: provide tutorials and guides throughout the game
//Relation: all functions integrate with ncurses UI
void showPagedGuide(const std::string& title,
                    const std::vector<std::vector<std::string> >& pages,
                    bool hasColor);
void showPreGameQuickGuide(bool hasColor);
void showFirstTimeTutorial(TutorialTopic topic, bool hasColor);
void showFullGuide(const Board& board, const RuleSet& rules, bool sabotageUnlocked, bool hasColor);
bool showQuitConfirmation(bool hasColor);
void showSabotageUnlockAnimation(bool hasColor);
void showSabotageTutorial(bool hasColor);
