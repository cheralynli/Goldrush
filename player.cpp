#include "player.hpp"

#include <cctype>

char tokenForName(const std::string& name, int index) {
    if (!name.empty()) {
        char c = name[0];
        if (c >= 'a' && c <= 'z') {
            c = static_cast<char>(c - 'a' + 'A');
        }
        return c;
    }
    return static_cast<char>('A' + index);
}

int totalWorth(const Player& player) {
    int total = player.cash + player.kids * 20000;
    if (player.hasHouse) {
        total += 100000;
    }
    return total;
}

std::string playerTypeLabel(PlayerType type) {
    return type == PlayerType::CPU ? "CPU" : "Human";
}

std::string cpuDifficultyLabel(CpuDifficulty difficulty) {
    switch (difficulty) {
        case CpuDifficulty::Easy:
            return "Easy";
        case CpuDifficulty::Hard:
            return "Hard";
        case CpuDifficulty::Normal:
        default:
            return "Normal";
    }
}

PlayerType playerTypeFromText(const std::string& text) {
    if (text.empty()) {
        return PlayerType::Human;
    }
    char c = static_cast<char>(std::toupper(static_cast<unsigned char>(text[0])));
    return c == 'C' ? PlayerType::CPU : PlayerType::Human;
}

CpuDifficulty cpuDifficultyFromText(const std::string& text) {
    if (text.empty()) {
        return CpuDifficulty::Normal;
    }
    char c = static_cast<char>(std::toupper(static_cast<unsigned char>(text[0])));
    if (c == 'E' || c == '1') {
        return CpuDifficulty::Easy;
    }
    if (c == 'H' || c == '3') {
        return CpuDifficulty::Hard;
    }
    return CpuDifficulty::Normal;
}
