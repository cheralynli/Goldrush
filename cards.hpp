#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "deck.hpp"
#include "random_service.hpp"
#include "rules.hpp"

//Purpose: categorizes card types
//Relation: used in card structs to identify type
enum CardCategory {
    CARD_ACTION,
    CARD_CAREER,
    CARD_HOUSE,
    CARD_INVEST,
    CARD_PET
};

//Purpose: identifies deck slots for serialization/restoration
//Relation: used in DeckManager
enum DeckSlot {
    DECK_ACTION,
    DECK_COLLEGE_CAREER,
    DECK_CAREER,
    DECK_HOUSE,
    DECK_INVEST,
    DECK_PET
};

//Purpose: defines possible effects of action cards
//Relation: used in ActionEffect
enum ActionEffectKind {
    ACTION_NO_EFFECT,
    ACTION_GAIN_CASH,
    ACTION_PAY_CASH,
    ACTION_GAIN_PER_KID,
    ACTION_PAY_PER_KID,
    ACTION_GAIN_SALARY_BONUS,
    ACTION_BONUS_IF_MARRIED,
    ACTION_MOVE_SPACES,
    ACTION_DUEL_MINIGAME
};

//Purpose: defines roll/spin conditions
//Relation: used in RollCondition
enum RollConditionKind {
    ROLL_ANY,
    ROLL_ODD,
    ROLL_EVEN,
    ROLL_RANGE,
    ROLL_EXACT
};

//Fields: kind, minValue, maxValue, exactValue
//Purpose: defines roll/spin condition
//Relation: used in ActionRollOutcome
struct RollCondition {
    RollConditionKind kind;
    int minValue;
    int maxValue;
    int exactValue;
};

//Fields: kind, amount, useTileValue
//Purpose: defines effect of an action card
//Relation: used in ActionCard
struct ActionEffect {
    ActionEffectKind kind;
    int amount;
    bool useTileValue;
};

//Fields: condition, effect, text
//Purpose: defines outcome of a roll-based action card
//Relation: used in ActionCard
struct ActionRollOutcome {
    RollCondition condition;
    ActionEffect effect;
    std::string text;
};

//Fields: id, title, category, description, effect, rollOutcomes, keepAfterUse
//Purpose: defines action card
//Relation: managed by DeckManager
struct ActionCard {
    std::string id;
    std::string title;
    CardCategory category;
    std::string description;
    ActionEffect effect;
    std::vector<ActionRollOutcome> rollOutcomes;
    bool keepAfterUse;
};

//Fields: id, title, category, description, salary, requiresDegree, keepAfterUse
//Purpose: defines career card
//Relation: managed by DeckManager
struct CareerCard {
    std::string id;
    std::string title;
    CardCategory category;
    std::string description;
    int salary;
    bool requiresDegree;
    bool keepAfterUse;
};

//Fields: id, title, category, description, cost, saleValue, keepAfterUse
//Purpose: defines house card
//Relation: managed by DeckManager
struct HouseCard {
    std::string id;
    std::string title;
    CardCategory category;
    std::string description;
    int cost;
    int saleValue;
    bool keepAfterUse;
};

//Fields: id, title, category, description, number, payout, keepAfterUse
//Purpose: defines investment card
//Relation: managed by DeckManager
struct InvestCard {
    std::string id;
    std::string title;
    CardCategory category;
    std::string description;
    int number;
    int payout;
    bool keepAfterUse;
};

//Fields: id, title, category, description, endValue, keepAfterUse
//Purpose: defines pet card
//Relation: managed by DeckManager
struct PetCard {
    std::string id;
    std::string title;
    CardCategory category;
    std::string description;
    int endValue;
    bool keepAfterUse;
};

//Fields: drawIds, discardIds
//Purpose: stores deck state for save/load.
//Relation: used in DeckManager::deckState and restoreDeckState
struct SerializedDeckState {
    std::vector<std::string> drawIds;
    std::vector<std::string> discardIds;
};

class DeckManager {
public:
    //constructer
    DeckManager(const RuleSet& rules, RandomService& random);
    //reset decks
    void reset(const RuleSet& rules, bool reshuffle = true);
    //get current rules
    const RuleSet& rules() const;
    //manage action cards
    bool drawActionCard(ActionCard& card);
    //manage action cards
    void resolveActionCard(const ActionCard& card, bool keepCard);
    //manage career cards
    std::vector<CareerCard> drawCareerChoices(bool requiresDegree, int count);
    //manage career cards
    void resolveCareerChoices(bool requiresDegree,
                              const std::vector<CareerCard>& choices,
                              int keptIndex);
    //draw other cards
    bool drawHouseCard(HouseCard& card);
    //draw other cards
    bool drawInvestCard(InvestCard& card);
    //draw other cards
    bool drawPetCard(PetCard& card);
    //serialize deck state
    SerializedDeckState deckState(DeckSlot slot) const;
    //restore deck state
    bool restoreDeckState(DeckSlot slot,
                          const SerializedDeckState& state,
                          std::string& error);

private:
    RuleSet ruleset;
    Deck<ActionCard> actionDeck;
    Deck<CareerCard> collegeCareerDeck;
    Deck<CareerCard> careerDeck;
    Deck<HouseCard> houseDeck;
    Deck<InvestCard> investDeck;
    Deck<PetCard> petDeck;

    //initialize decks
    void initDecks(bool reshuffle = true);
    void initActionDeck(bool reshuffle);
    void initCareerDecks(bool reshuffle);
    void initHouseDeck(bool reshuffle);
    void initInvestDeck(bool reshuffle);
    void initPetDeck(bool reshuffle);
};

//checks if card uses roll outcomes
bool actionCardUsesRoll(const ActionCard& card);
//evaluates roll condition
bool matchesRollCondition(const RollCondition& condition, int roll);
//finds matching outcome
const ActionRollOutcome* findMatchingRollOutcome(const ActionCard& card, int roll);
//human-readable description
std::string describeRollCondition(const RollCondition& condition);
