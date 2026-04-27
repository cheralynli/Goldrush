#pragma once

#include <string>

struct PongMinigameResult {
    int playerScore;
    int cpuScore;
    bool abandoned;
};

struct PongDuelResult {
    int winnerSide; // 0 = left, 1 = right, -1 = none
    bool abandoned;
};

PongMinigameResult playPongMinigame(const std::string& playerName, bool hasColor);
PongDuelResult playPongDuelMinigame(const std::string& leftPlayerName,
                                    const std::string& rightPlayerName,
                                    bool hasColor,
                                    bool rightSideCpu);
