#include "cards.hpp"

#include <algorithm>
#include <ctime>

namespace {
template <typename T>
void fillDeck(std::vector<T>& deck,
              const std::vector<T>& prototypes,
              int desiredCount,
              std::mt19937& rng) {
    deck.clear();
    if (desiredCount <= 0 || prototypes.empty()) {
        return;
    }

    while (static_cast<int>(deck.size()) < desiredCount) {
        for (size_t i = 0; i < prototypes.size() && static_cast<int>(deck.size()) < desiredCount; ++i) {
            deck.push_back(prototypes[i]);
        }
    }
    std::shuffle(deck.begin(), deck.end(), rng);
}

template <typename T>
T drawNext(std::vector<T>& deck, size_t& index, std::mt19937& rng) {
    if (deck.empty()) {
        return T();
    }
    if (index >= deck.size()) {
        std::shuffle(deck.begin(), deck.end(), rng);
        index = 0;
    }
    return deck[index++];
}
}

DeckManager::DeckManager(const RuleSet& rules)
    : ruleset(rules),
      rng(static_cast<unsigned>(std::time(nullptr))),
      actionIndex(0),
      collegeCareerIndex(0),
      careerIndex(0),
      houseIndex(0),
      investIndex(0),
      petIndex(0) {
    initDecks();
}

void DeckManager::reset(const RuleSet& rules) {
    ruleset = rules;
    actionIndex = 0;
    collegeCareerIndex = 0;
    careerIndex = 0;
    houseIndex = 0;
    investIndex = 0;
    petIndex = 0;
    initDecks();
}

const RuleSet& DeckManager::rules() const {
    return ruleset;
}

ActionCard DeckManager::drawActionCard() {
    return drawNext(actionDeck, actionIndex, rng);
}

std::vector<CareerCard> DeckManager::drawCareerChoices(bool requiresDegree, int count) {
    std::vector<CareerCard> choices;
    choices.reserve(count);

    std::vector<CareerCard>& deck = requiresDegree ? collegeCareerDeck : careerDeck;
    size_t& index = requiresDegree ? collegeCareerIndex : careerIndex;

    for (int i = 0; i < count; ++i) {
        choices.push_back(drawNext(deck, index, rng));
    }
    return choices;
}

HouseCard DeckManager::drawHouseCard() {
    return drawNext(houseDeck, houseIndex, rng);
}

InvestCard DeckManager::drawInvestCard() {
    return drawNext(investDeck, investIndex, rng);
}

PetCard DeckManager::drawPetCard() {
    return drawNext(petDeck, petIndex, rng);
}

void DeckManager::initDecks() {
    initActionDeck();
    initCareerDecks();
    initHouseDeck();
    initInvestDeck();
    initPetDeck();
}

void DeckManager::initActionDeck() {
    std::vector<ActionCard> prototypes;
    prototypes.push_back({"Tax Refund", "Collect a surprise refund from the bank.", ACTION_GAIN_CASH, 25000});
    prototypes.push_back({"Car Trouble", "Pay for a major repair bill.", ACTION_PAY_CASH, 20000});
    prototypes.push_back({"Family Picnic", "Collect $10K per child for the photo shoot.", ACTION_GAIN_PER_KID, 10000});
    prototypes.push_back({"School Supplies", "Pay $8K per child.", ACTION_PAY_PER_KID, 8000});
    prototypes.push_back({"Side Hustle", "Your side gig boosts your salary by $10K.", ACTION_GAIN_SALARY_BONUS, 10000});
    prototypes.push_back({"Anniversary Bonus", "If married, collect $30K.", ACTION_BONUS_IF_MARRIED, 30000});
    prototypes.push_back({"Lucky Day", "Collect a windfall from the bank.", ACTION_GAIN_CASH, 40000});
    prototypes.push_back({"Unexpected Bill", "Pay an emergency expense.", ACTION_PAY_CASH, 30000});
    fillDeck(actionDeck, prototypes, ruleset.components.actionCards, rng);
}

void DeckManager::initCareerDecks() {
    std::vector<CareerCard> collegePrototypes;
    collegePrototypes.push_back({"Doctor", "Long degree path, strong salary.", 80000, true});
    collegePrototypes.push_back({"Architect", "Design work with steady pay.", 70000, true});
    collegePrototypes.push_back({"Scientist", "Research role with growth.", 75000, true});
    collegePrototypes.push_back({"Engineer", "Problem solving career.", 72000, true});
    collegePrototypes.push_back({"Professor", "Academic path with good pay.", 68000, true});

    std::vector<CareerCard> careerPrototypes;
    careerPrototypes.push_back({"Chef", "Fast-paced kitchen career.", 42000, false});
    careerPrototypes.push_back({"Photographer", "Creative work on the go.", 38000, false});
    careerPrototypes.push_back({"Mechanic", "Hands-on technical career.", 43000, false});
    careerPrototypes.push_back({"Sales Rep", "People-first role with commissions.", 41000, false});
    careerPrototypes.push_back({"Developer", "Technical career with steady income.", 46000, false});

    fillDeck(collegeCareerDeck, collegePrototypes, ruleset.components.collegeCareerCards, rng);
    fillDeck(careerDeck, careerPrototypes, ruleset.components.careerCards, rng);
}

void DeckManager::initHouseDeck() {
    std::vector<HouseCard> prototypes;
    prototypes.push_back({"Lake Cabin", 50000, 90000});
    prototypes.push_back({"Townhouse", 60000, 95000});
    prototypes.push_back({"Hilltop Villa", 80000, 130000});
    prototypes.push_back({"Starter Home", 45000, 75000});
    prototypes.push_back({"Beach House", 90000, 140000});
    fillDeck(houseDeck, prototypes, ruleset.components.houseCards, rng);
}

void DeckManager::initInvestDeck() {
    std::vector<InvestCard> prototypes;
    prototypes.push_back({"Invest on 3", 3, ruleset.investmentMatchPayout});
    prototypes.push_back({"Invest on 5", 5, ruleset.investmentMatchPayout});
    prototypes.push_back({"Invest on 7", 7, ruleset.investmentMatchPayout});
    prototypes.push_back({"Invest on 9", 9, ruleset.investmentMatchPayout});
    fillDeck(investDeck, prototypes, ruleset.components.investCards, rng);
}

void DeckManager::initPetDeck() {
    std::vector<PetCard> prototypes;
    prototypes.push_back({"Dog", "A loyal family dog joins the car.", 100000});
    prototypes.push_back({"Cat", "A calm cat settles into the household.", 100000});
    prototypes.push_back({"Rabbit", "A tiny pet with a big personality.", 100000});
    prototypes.push_back({"Parrot", "A chatty bird becomes part of the trip.", 100000});
    fillDeck(petDeck, prototypes, ruleset.components.petCards, rng);
}
