#include "game.hpp"
#include "tutorials.h"
#include "ui.h"
#include "ui_helpers.h"

#include <algorithm>
#include <string>

void Game::showTutorial() {
    if (!rules.toggles.tutorialEnabled) {
        return;
    }
    showPreGameQuickGuide(hasColor);
    addHistory("Quick guide reviewed");
}

void Game::showGuidePopup() const {
    showFullGuide(board,
                  rules,
                  sabotageUnlockAnnounced,
                  hasColor);
}

void Game::resetTutorialFlags() {
    tutorialFlags = TutorialFlags();
    sabotageUnlockAnnounced = false;
}

void Game::maybeShowFirstTimeTutorial(TutorialTopic topic) {
    if (!rules.toggles.tutorialEnabled) {
        return;
    }
    bool& seen = tutorialFlagForTopic(tutorialFlags, topic);
    if (seen) {
        return;
    }
    showFirstTimeTutorial(topic, hasColor);
    seen = true;
}

void Game::maybeShowLoanTutorial(int playerIndex, const PaymentResult& payment) {
    if (payment.loansTaken <= 0 || isCpuPlayer(playerIndex)) {
        return;
    }
    maybeShowFirstTimeTutorial(TutorialTopic::AutomaticLoan);
}

bool Game::isSabotageUnlockedForPlayer(int playerIndex) const {
    if (playerIndex < 0 || playerIndex >= static_cast<int>(players.size())) {
        return false;
    }
    const int unlockTurn = std::max(1, settings.sabotageUnlockTurn);
    return players[static_cast<std::size_t>(playerIndex)].turnsTaken + 1 >= unlockTurn;
}

void Game::maybeShowSabotageUnlock(int playerIndex) {
    if (sabotageUnlockAnnounced || !isSabotageUnlockedForPlayer(playerIndex)) {
        return;
    }
    
    const bool previousAutoAdvance = autoAdvanceUi;
    autoAdvanceUi = isCpuPlayer(playerIndex);

    showSabotageUnlockAnimation(hasColor);
    showSabotageTutorial(hasColor);
    
    //TODO number 1 - fix blackscreen
    // Force refresh all existing windows
    if (titleWin) { touchwin(titleWin); wrefresh(titleWin); }
    if (boardWin) { touchwin(boardWin); wrefresh(boardWin); }
    if (infoWin) { touchwin(infoWin); wrefresh(infoWin); }
    if (msgWin) { touchwin(msgWin); wrefresh(msgWin); }


    renderGame(playerIndex,
               players[playerIndex].name + "'s turn - Sabotage Unlocked!",
               "Press ENTER to continue...");
    if (msgWin) {
        draw_message_ui(msgWin,
                        players[playerIndex].name + "'s turn - Sabotage Unlocked!",
                        "Press ENTER to continue...");
    }

    autoAdvanceUi = previousAutoAdvance;

    tutorialFlagForTopic(tutorialFlags, TutorialTopic::Sabotage) = true;
    sabotageUnlockAnnounced = true;
    addHistory("Sabotage unlocked from Turn " + std::to_string(std::max(1, settings.sabotageUnlockTurn)));
}

