#pragma once

#include <string>
#include <vector>

struct Player {
    std::string name;
    char token;
    int tile;
    int cash;
    std::string job;
    int salary;
    bool married;
    int kids;
    bool collegeGraduate;
    bool usedNightSchool;
    bool hasHouse;
    std::string houseName;
    int houseValue;
    int loans;
    int investedNumber;
    int investPayout;
    int spinToWinTokens;
    int retirementPlace;
    int retirementBonus;
    int finalHouseSaleValue;
    std::string retirementHome;
    std::vector<std::string> actionCards;
    std::vector<std::string> petCards;
    bool retired;
    int startChoice;
    int familyChoice;
    int riskChoice;
};

char tokenForName(const std::string& name, int index);
int totalWorth(const Player& player);
