#pragma once

#include <cstddef>
#include <vector>

#include "random_service.hpp"

//Purpose: generic deck manager for card-like objects (any type T).
//Relation: used to handle draw/discard piles in your game (Action cards, Career cards, Pet cards, etc.), with randomness provided by RandomService.
template <typename T>
class Deck {
public:

    //Input: none
    //Output: initializes deck with no bound random service
    //Purpose: creates an empty deck without shuffle capability until bound
    //Relation: requires later call to bindRandom.
    Deck()
        : random(nullptr) {
    }

    //Input: reference to RandomService
    //Output: initializes deck with bound random service
    //Purpose: creates deck ready to shuffle
    //Relation: ties deck operations to deterministic/randomized behavior.
    explicit Deck(RandomService& randomService)
        : random(&randomService) {
    }

    //Input: reference to RandomService
    //Output: binds deck to random service
    //Purpose: enables shuffle functionality
    //Relation: required if default constructor used.
    void bindRandom(RandomService& randomService) {
        random = &randomService;
    }

    //Input: vector of cards, reshuffle flag
    //Output: resets draw pile, clears discard pile, optionally shuffles
    //Purpose: reinitializes deck state
    //Relation: used at game start or after reset events.
    void reset(const std::vector<T>& cards, bool reshuffle = true) {
        drawPile = cards;
        discardPile.clear();
        if (reshuffle) {
            shuffle();
        }
    }

    //Input: none
    //Output: shuffles draw pile if random bound
    //Purpose: randomizes card order
    //Relation: depends on RandomService.
    void shuffle() {
        if (random == nullptr) {
            return;
        }
        random->shuffle(drawPile);
    }

    //Input: reference to card variable
    //Output: true if card drawn, false if empty
    //Purpose: draws top card from draw pile, replenishes from discard if needed
    //Relation: core gameplay mechanic for card usage.
    bool draw(T& card) {
        replenishIfNeeded();
        if (drawPile.empty()) {
            return false;
        }

        card = drawPile.back();
        drawPile.pop_back();
        return true;
    }

    //Input: card object
    //Output: adds card to discard pile
    //Purpose: tracks used cards
    //Relation: supports recycling via replenishIfNeeded.
    void discard(const T& card) {
        discardPile.push_back(card);
    }

    //Input: draw pile, discard pile vectors
    //Output: sets deck state directly
    //Purpose: restores deck from saved state
    //Relation: supports save/load functionality.
    void setState(const std::vector<T>& drawCards, const std::vector<T>& discardCards) {
        drawPile = drawCards;
        discardPile = discardCards;
    }

    //Input: none
    //Output: true if both piles empty
    //Purpose: checks if deck exhausted
    //Relation: used to detect end conditions.
    bool empty() const {
        return drawPile.empty() && discardPile.empty();
    }

    //Input: none
    //Output: number of cards in draw pile
    //Purpose: reports deck size
    //Relation: used for UI or logic checks.
    std::size_t size() const {
        return drawPile.size();
    }

    //Input: none
    //Output: number of cards in discard pile
    //Purpose: reports discard size
    //Relation: used for debugging or balancing.
    std::size_t discardSize() const {
        return discardPile.size();
    }

    //Input: none
    //Output: pointer to top card or nullptr
    //Purpose: allows inspection without drawing
    //Relation: used for preview mechanics.
    const T* peek() const {
        if (drawPile.empty()) {
            return nullptr;
        }
        return &drawPile.back();
    }

    //Input: number of cards to preview
    //Output: vector of top cards
    //Purpose: allows inspection of multiple upcoming cards
    //Relation: supports advanced mechanics (e.g., “look ahead” actions).
    std::vector<T> peek(std::size_t count) const {
        std::vector<T> cards;
        if (count == 0 || drawPile.empty()) {
            return cards;
        }

        if (count > drawPile.size()) {
            count = drawPile.size();
        }

        cards.reserve(count);
        for (std::size_t i = 0; i < count; ++i) {
            cards.push_back(drawPile[drawPile.size() - 1 - i]);
        }
        return cards;
    }

    //Input: none
    //Output: reference to draw pile vector
    //Purpose: exposes current draw pile
    //Relation: used for serialization or debugging.
    const std::vector<T>& drawCards() const {
        return drawPile;
    }

    //Input: none
    //Output: reference to discard pile vector
    //Purpose: exposes current discard pile
    //Relation: used for serialization or debugging.
    const std::vector<T>& discardCards() const {
        return discardPile;
    }

private:

    //Input: none
    //Output: swaps discard pile into draw pile if draw empty, then shuffles
    //Purpose: ensures deck continuity
    //Relation: called automatically in draw.
    void replenishIfNeeded() {
        if (!drawPile.empty() || discardPile.empty()) {
            return;
        }

        drawPile.swap(discardPile);
        shuffle();
    }

    RandomService* random;
    std::vector<T> drawPile;
    std::vector<T> discardPile;
};
