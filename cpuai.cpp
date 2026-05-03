#include "game.hpp"
#include "cpu_player.hpp"
#include "ui.h"
#include "ui_helpers.h"

#include <string>

std::string cpuReasonForAction(const Player& player, const std::string& action);

void Game::showCpuThinking(int playerIndex, const std::string& action) const {
    if (!isCpuPlayer(playerIndex) || !msgWin) {
        return;
    }

    const Player& player = players[static_cast<std::size_t>(playerIndex)];
    const std::string detail =
        "CPU " + cpuDifficultyLabel(player.cpuDifficulty) + " | " + action +
        " | " + cpuReasonForAction(player, action);
    renderGame(playerIndex, player.name + " is thinking...", detail);
    napms(autoAdvanceUi ? 300 : 900);
}

std::string cpuReasonForAction(const Player& player, const std::string& action) {
    switch (player.cpuDifficulty) {
        case CpuDifficulty::Easy:
            return "Easy CPU uses simple, sometimes random choices.";
        case CpuDifficulty::Hard:
            if (action.find("Career") != std::string::npos ||
                action.find("career") != std::string::npos) {
                return "Hard CPU favors the highest salary or strongest long-term option available.";
            }
            if (action.find("Risk") != std::string::npos ||
                action.find("risky") != std::string::npos) {
                return "Hard CPU weighs cash, loans, and route upside before choosing.";
            }
            if (action.find("sabotage") != std::string::npos ||
                action.find("Sabotage") != std::string::npos) {
                return "Hard CPU targets the player with the strongest visible position.";
            }
            return "Hard CPU picks the option with the best expected long-term value.";
        case CpuDifficulty::Normal:
        default:
            return "Normal CPU chooses a practical option with a basic money or progress benefit.";
    }
}

void Game::maybeCpuSabotage(int playerIndex) {
    if (!isCpuPlayer(playerIndex) ||
        !isSabotageUnlockedForPlayer(playerIndex) ||
        !cpu.shouldUseSabotage(players[playerIndex], players[playerIndex].turnsTaken)) {
        return;
    }

    const int targetIndex = cpu.chooseSabotageTarget(players[playerIndex], players, playerIndex);
    if (targetIndex < 0) {
        return;
    }

    const SabotageType type = cpu.chooseSabotageType(players[playerIndex],
                                                    players[static_cast<std::size_t>(targetIndex)],
                                                    players[playerIndex].turnsTaken);
    const bool previousAutoAdvance = autoAdvanceUi;
    autoAdvanceUi = true;
    showCpuThinking(playerIndex,
                    "CPU sabotage: " + sabotageTypeName(type) + " on " +
                    players[static_cast<std::size_t>(targetIndex)].name);
    executeSabotage(playerIndex, targetIndex, type);
    autoAdvanceUi = previousAutoAdvance;
}

