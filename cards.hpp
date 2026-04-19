#pragma once

#include <random>
#include <string>
#include <vector>

#include "rules.hpp"

enum ActionEffectKind {
    ACTION_GAIN_CASH,
    ACTION_PAY_CASH,
    ACTION_GAIN_PER_KID,
    ACTION_PAY_PER_KID,
    ACTION_GAIN_SALARY_BONUS,
    ACTION_BONUS_IF_MARRIED
};

struct ActionCard {
    std::string title;
    std::string description;
    ActionEffectKind effect;
    int amount;
};

struct CareerCard {
    std::string title;
    std::string description;
    int salary;
    bool requiresDegree;
};

struct HouseCard {
    std::string title;
    int cost;
    int saleValue;
};

struct InvestCard {
    std::string title;
    int number;
    int payout;
};

struct PetCard {
    std::string title;
    std::string description;
    int endValue;
};

class DeckManager {
public:
    explicit DeckManager(const RuleSet& rules);

    void reset(const RuleSet& rules);
    const RuleSet& rules() const;

    ActionCard drawActionCard();
    std::vector<CareerCard> drawCareerChoices(bool requiresDegree, int count);
    HouseCard drawHouseCard();
    InvestCard drawInvestCard();
    PetCard drawPetCard();

private:
    RuleSet ruleset;
    std::mt19937 rng;

    std::vector<ActionCard> actionDeck;
    std::vector<CareerCard> collegeCareerDeck;
    std::vector<CareerCard> careerDeck;
    std::vector<HouseCard> houseDeck;
    std::vector<InvestCard> investDeck;
    std::vector<PetCard> petDeck;

    size_t actionIndex;
    size_t collegeCareerIndex;
    size_t careerIndex;
    size_t houseIndex;
    size_t investIndex;
    size_t petIndex;

    void initDecks();
    void initActionDeck();
    void initCareerDecks();
    void initHouseDeck();
    void initInvestDeck();
    void initPetDeck();
};
