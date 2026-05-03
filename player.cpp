#include "player.hpp"

#include <cctype>

//Input: player name string, fallback index
//Output: single character token (first letter of name, capitalized; or fallback letter 'A' + index)
//Purpose: generates a short token/marker for a player based on their name
//Relation: used for labeling players in UI or game state when a full name isn’t practical.
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

//Input: Player object (cash, kids, house, retirement bonus, action cards, pet cards, loans)
//Output: integer total worth of player
//Purpose: calculates final net worth at end of game, including assets and penalties
//Relation: used in scoring and determining winner at retirement; depends on Player struct fields.
int totalWorth(const Player& player) {
    int total = player.cash + player.kids * 50000;
    if (player.hasHouse) {
        total += player.finalHouseSaleValue > 0 ? player.finalHouseSaleValue : player.houseValue;
    }
    total += player.retirementBonus;
    total += static_cast<int>(player.actionCards.size()) * 100000;
    total += static_cast<int>(player.petCards.size()) * 100000;
    total -= player.loans * 60000;
    return total;
}

//Input: PlayerType enum (CPU or Human)
//Output: string label "CPU" or "Human"
//Purpose: provides human-readable label for player type
//Relation: used in UI or logs to distinguish between human and CPU players.
std::string playerTypeLabel(PlayerType type) {
    return type == PlayerType::CPU ? "CPU" : "Human";
}

//Input: CpuDifficulty enum (Easy, Normal, Hard)
//Output: string label "Easy", "Normal", or "Hard"
//Purpose: converts difficulty enum to readable text
//Relation: used in menus, status lines, or debugging to show CPU difficulty level.
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

//Input: text string (first character checked)
//Output: PlayerType::CPU if starts with 'C', otherwise PlayerType::Human
//Purpose: parses text input into player type
//Relation: supports configuration or command-line parsing for player setup.
PlayerType playerTypeFromText(const std::string& text) {
    if (text.empty()) {
        return PlayerType::Human;
    }
    char c = static_cast<char>(std::toupper(static_cast<unsigned char>(text[0])));
    return c == 'C' ? PlayerType::CPU : PlayerType::Human;
}

//Input: text string (first character checked)
//Output: CpuDifficulty::Easy if starts with 'E' or '1'; Hard if 'H' or '3'; otherwise Normal
//Purpose: parses text input into CPU difficulty level
//Relation: supports configuration or command-line parsing for CPU setup.
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
