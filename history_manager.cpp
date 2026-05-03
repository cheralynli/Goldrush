#include "game.hpp"
#include "completed_history.h"
#include "player.hpp"
#include "bank.hpp"

#include <algorithm>
#include <ctime>
#include <sstream>
#include <string>


std::string formatCompletedTime(std::time_t timestamp) {
    std::tm localTime;
#if defined(_WIN32)
    if (localtime_s(&localTime, &timestamp) != 0) {
        return "unknown";
    }
#else
    if (localtime_r(&timestamp, &localTime) == nullptr) {
        return "unknown";
    }
#endif

    char buffer[64] = {0};
    if (std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &localTime) == 0) {
        return "unknown";
    }
    return buffer;
}

std::string playerCompletionSummary(const Player& player, int finalWorth, const Bank& bank) {
    std::ostringstream out;
    out << player.name << " " << playerTypeLabel(player.type);
    if (player.type == PlayerType::CPU) {
        out << "(" << cpuDifficultyLabel(player.cpuDifficulty) << ")";
    }
    out << " net $" << finalWorth
        << " cash $" << player.cash
        << " loans $" << bank.totalLoanDebt(player)
        << " turns " << player.turnsTaken;
    if (!player.job.empty() && player.job != "Unemployed") {
        out << " job " << player.job;
    }
    if (player.hasHouse || !player.houseName.empty()) {
        out << " house " << player.houseName << " $" <<
            (player.finalHouseSaleValue > 0 ? player.finalHouseSaleValue : player.houseValue);
    }
    return out.str();
}

void Game::appendCompletedGameHistoryEntry(int winnerIndex, int winnerScore) {
    if (winnerIndex < 0 || winnerIndex >= static_cast<int>(players.size())) {
        return;
    }

    CompletedGameEntry entry;
    entry.date = formatCompletedTime(std::time(nullptr));
    entry.gameId = gameId.empty() ? "UNSAVED_GAME" : gameId;
    entry.winner = players[static_cast<std::size_t>(winnerIndex)].name;
    entry.winnerScore = winnerScore;
    entry.winnerCash = players[static_cast<std::size_t>(winnerIndex)].cash;
    entry.winnerNetWorth = winnerScore;
    entry.rounds = turnCounter;
    entry.mode = rules.editionName;
    entry.settings = gameSettingsSummary(settings);

    std::ostringstream compactPlayers;
    std::ostringstream detailedPlayers;
    std::vector<int> ranking;
    ranking.reserve(players.size());
    for (std::size_t i = 0; i < players.size(); ++i) {
        ranking.push_back(static_cast<int>(i));
        const int finalWorth = calculateFinalWorth(players[i]);
        if (i > 0) {
            compactPlayers << ",";
            detailedPlayers << " | ";
        }
        compactPlayers << players[i].name << ":" << finalWorth;
        detailedPlayers << playerCompletionSummary(players[i], finalWorth, bank);
    }
    entry.players = compactPlayers.str();
    entry.playerDetails = detailedPlayers.str();

    std::sort(ranking.begin(), ranking.end(), [this](int left, int right) {
        const int leftWorth = calculateFinalWorth(players[static_cast<std::size_t>(left)]);
        const int rightWorth = calculateFinalWorth(players[static_cast<std::size_t>(right)]);
        if (leftWorth != rightWorth) {
            return leftWorth > rightWorth;
        }
        return players[static_cast<std::size_t>(left)].cash >
               players[static_cast<std::size_t>(right)].cash;
    });

    entry.detailLines.push_back("Winner: " + entry.winner +
                                " with final net worth $" + std::to_string(winnerScore));
    if (ranking.size() > 1) {
        const int runnerUpWorth = calculateFinalWorth(players[static_cast<std::size_t>(ranking[1])]);
        entry.detailLines.push_back("Winning margin: $" + std::to_string(winnerScore - runnerUpWorth));
    }
    entry.detailLines.push_back("Game: " + entry.gameId + " | Completed: " + entry.date);
    entry.detailLines.push_back("Mode: " + entry.mode);
    entry.detailLines.push_back("Settings: " + entry.settings);
    entry.detailLines.push_back("Turns: " + std::to_string(entry.rounds));
    entry.detailLines.push_back("");
    entry.detailLines.push_back("FINAL RANKING");
    for (std::size_t rank = 0; rank < ranking.size(); ++rank) {
        const Player& player = players[static_cast<std::size_t>(ranking[rank])];
        entry.detailLines.push_back("#" + std::to_string(rank + 1) + " " +
                                    player.name + " - $" +
                                    std::to_string(calculateFinalWorth(player)));
    }
    entry.detailLines.push_back("");
    entry.detailLines.push_back("PLAYER BREAKDOWNS");
    for (std::size_t rank = 0; rank < ranking.size(); ++rank) {
        const Player& player = players[static_cast<std::size_t>(ranking[rank])];
        const int finalWorth = calculateFinalWorth(player);
        const int debt = bank.totalLoanDebt(player);
        const int houseValue = player.finalHouseSaleValue > 0
            ? player.finalHouseSaleValue
            : player.houseValue;
        const int actionBonus = static_cast<int>(player.actionCards.size()) * 100000;
        const int petBonus = static_cast<int>(player.petCards.size()) * 100000;
        const int kidBonus = player.kids * 50000;

        entry.detailLines.push_back("#" + std::to_string(rank + 1) + " " +
                                    player.name + " net $" + std::to_string(finalWorth));
        entry.detailLines.push_back("  Cash $" + std::to_string(player.cash) +
                                    " | Debt/loans -$" + std::to_string(debt) +
                                    " (" + std::to_string(player.loans) + " loans)");
        entry.detailLines.push_back("  Job " +
                                    (player.job.empty() ? "Unemployed" : player.job) +
                                    " | Salary $" + std::to_string(player.salary));
        entry.detailLines.push_back("  House " +
                                    (player.houseName.empty() ? "none" : player.houseName) +
                                    " | Value $" + std::to_string(houseValue));
        if (player.investedNumber > 0 && player.investPayout > 0) {
            entry.detailLines.push_back("  Investment spinner " +
                                        std::to_string(player.investedNumber) +
                                        " | Match payout $" +
                                        std::to_string(player.investPayout));
        } else {
            entry.detailLines.push_back("  Investments none recorded");
        }
        entry.detailLines.push_back("  Kids " + std::to_string(player.kids) +
                                    " bonus $" + std::to_string(kidBonus) +
                                    " | Pets " + std::to_string(player.petCards.size()) +
                                    " bonus $" + std::to_string(petBonus));
        entry.detailLines.push_back("  Action cards " +
                                    std::to_string(player.actionCards.size()) +
                                    " bonus $" + std::to_string(actionBonus) +
                                    " | Retirement bonus $" +
                                    std::to_string(player.retirementBonus));
        entry.detailLines.push_back("  Married " +
                                    std::string(player.married ? "yes" : "no") +
                                    " | Retirement " +
                                    (player.retirementHome.empty() ? "not recorded" : player.retirementHome));
        if (player.sabotageDebt > 0 ||
            player.shieldCards > 0 ||
            player.insuranceUses > 0 ||
            player.spinToWinTokens > 0) {
            entry.detailLines.push_back("  Other tracked: sabotage debt $" +
                                        std::to_string(player.sabotageDebt) +
                                        ", shields " + std::to_string(player.shieldCards) +
                                        ", insurance " + std::to_string(player.insuranceUses) +
                                        ", spin tokens " + std::to_string(player.spinToWinTokens));
        }
    }

    std::string error;
    if (!appendCompletedGameHistory(entry, error) && !error.empty()) {
        addHistory("Could not write completed history: " + error);
    }
}

