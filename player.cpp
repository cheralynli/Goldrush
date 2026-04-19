#include "player.hpp"

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
