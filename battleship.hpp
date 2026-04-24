#pragma once

#include <string>

struct BattleshipMinigameResult {
    int shipsDestroyed;
    bool clearedWave;
    bool abandoned;
};

BattleshipMinigameResult playBattleshipMinigame(const std::string& playerName, bool hasColor);
