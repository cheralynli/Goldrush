#include "spins.hpp"

int marriageGiftFromSpin(int spin) {
    return spin <= 5 ? 50000 : 100000;
}

int babiesFromSpin(int spin) {
    if (spin <= 2) return 0;
    if (spin <= 6) return 1;
    if (spin <= 8) return 2;
    return 3;
}

std::string babiesLabel(int babies) {
    if (babies <= 0) return "No babies";
    if (babies == 1) return "1 baby";
    if (babies == 2) return "Twins";
    return "Triplets";
}

int houseSaleValueFromSpin(int baseValue, int spin) {
    return baseValue + (spin * 5000);
}

int retirementBonusForPlace(int place) {
    switch (place) {
        case 1: return 400000;
        case 2: return 300000;
        case 3: return 200000;
        case 4: return 100000;
        default: return 0;
    }
}

int safeRoutePayout(int spin) {
    return 15000 + (spin * 1000);
}

int riskyRoutePayout(int spin) {
    if (spin <= 2) return -100000;
    if (spin <= 5) return -50000;
    if (spin <= 7) return 50000;
    if (spin == 8) return 75000;
    if (spin == 9) return 100000;
    return 150000;
}
