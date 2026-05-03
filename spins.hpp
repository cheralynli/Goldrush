#pragma once

#include <string>

//Input: none
//Output: none
//Purpose: declares spin functions
//Relation: included in any module needing spins
int marriageGiftFromSpin(int spin);
int babiesFromSpin(int spin);
std::string babiesLabel(int babies);
int houseSaleValueFromSpin(int baseValue, int spin);
int retirementBonusForPlace(int place);
int safeRoutePayout(int spin);
int riskyRoutePayout(int spin);
