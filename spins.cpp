#include "spins.hpp"

//Input: int spin result
//Output: int marriage gift
//Purpose: determine value of marriage gift based on spinner
//Relation: called during marriageStop to award player
int marriageGiftFromSpin(int spin) {
    return spin <= 5 ? 50000 : 100000;
}

//Input: int spin result
//Output: int babies
//Purpose: determine number of babies born 
//Relation: called during babyEvent to determine number of babies
int babiesFromSpin(int spin) {
    if (spin <= 2) return 0;
    if (spin <= 6) return 1;
    if (spin <= 8) return 2;
    return 3;
}

//Input: int number of babies
//Output: string babiesLabel
//Purpose: inform player the number of babies
//Relation: called during babyEvent
std::string babiesLabel(int babies) {
    if (babies <= 0) return "No babies";
    if (babies == 1) return "1 baby";
    if (babies == 2) return "Twins";
    return "Triplets";
}

//Input: int baseValue, int spin
//Output: int houseSaleValueFromSpin
//Purpose: increase house sale value depending on spin
//Relation: called during calculating finalHouseSaleValue
int houseSaleValueFromSpin(int baseValue, int spin) {
    return baseValue + (spin * 5000);
}

//Input: int place
//Output: retirementBonusForPlace
//Puropse: calculation of retirement bonus based on player finishing order
//Relation: called during resolveRetirement
int retirementBonusForPlace(int place) {
    switch (place) {
        case 1: return 400000;
        case 2: return 300000;
        case 3: return 200000;
        case 4: return 100000;
        default: return 0;
    }
}

//Input: int spin
//Output: int safeRoutePayout
//Purpose: calculate bonus for safe route
//Relation: called during resolveSafeRoute
int safeRoutePayout(int spin) {
    return 15000 + (spin * 1000);
}

//Input: int spin
//Outpuut: int riskyRoutePayout
//Purpose: calculate bonus/penalty for risky route
//Relation: called during resolveRiskyRoute
int riskyRoutePayout(int spin) {
    if (spin <= 2) return -100000;
    if (spin <= 5) return -50000;
    if (spin <= 7) return 50000;
    if (spin == 8) return 75000;
    if (spin == 9) return 100000;
    return 150000;
}
