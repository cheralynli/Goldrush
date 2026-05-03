#pragma once

#include <algorithm>
#include <cstdint>
#include <random>
#include <sstream>
#include <string>
#include <vector>

class RandomService {
public:
    //Input: none
    //Output: initializes with a random seed from std::random_device
    //Purpose: creates a random engine with unpredictable seed for varied gameplay outcomes
    //Relation: used when you want non-deterministic randomness (different results each run).
    RandomService()
        : fixedSeed(false),
          seedValue(static_cast<std::uint32_t>(std::random_device()())),
          engine(seedValue) {
    }
    //Input: fixed seed value
    //Output: initializes with deterministic seed
    //Purpose: ensures reproducible randomness (same sequence each run)
    //Relation: useful for debugging or testing hidden test cases consistently.
    explicit RandomService(std::uint32_t seed)
        : fixedSeed(true),
          seedValue(seed),
          engine(seedValue) {
    }
    //Input: minimum and maximum integer bounds
    //Output: random integer in range [minValue, maxValue]
    //Purpose: general-purpose random number generator
    //Relation: core utility used by other functions (e.g., roll10).
    int uniformInt(int minValue, int maxValue) {
        std::uniform_int_distribution<int> distribution(minValue, maxValue);
        return distribution(engine);
    }
    //Input: none
    //Output: random integer between 1 and 10
    //Purpose: simulates a 10-sided die roll
    //Relation: shorthand wrapper around uniformInt(1, 10).
    int roll10() {
        return uniformInt(1, 10);
    }
    //Input: vector of values
    //Output: modifies vector order randomly
    //Purpose: randomizes card decks, token orders, or other collections
    //Relation: used in game systems where order matters (e.g., shuffling action cards).
    template <typename T>
    void shuffle(std::vector<T>& values) {
        if (values.size() < 2) {
            return;
        }
        std::shuffle(values.begin(), values.end(), engine);
    }
    //Input: integer value
    //Output: true if odd
    //Purpose: quick parity check
    //Relation: helper for rule logic or random outcomes.
    static bool isOdd(int value) {
        return (value % 2) != 0;
    }
    //Input: integer value
    //Output: true if even
    //Purpose: quick parity check
    //Relation: complements isOdd.
    static bool isEven(int value) {
        return (value % 2) == 0;
    }
    //Input: integer value, min, max
    //Output: true if within range
    //Purpose: bounds checking utility
    //Relation: can validate random results or rule constraints.
    static bool inRange(int value, int minValue, int maxValue) {
        return value >= minValue && value <= maxValue;
    }
    //Input: none
    //Output: string representation of current RNG engine state
    //Purpose: allows saving RNG state for later restoration
    //Relation: supports deterministic replay or saving/loading game sessions.
    std::string serializeState() const {
        std::ostringstream out;
        out << engine;
        return out.str();
    }
    //Input: serialized state string, fixed seed flag, seed value
    //Output: true if restoration succeeded
    //Purpose: restores RNG engine to a previous state
    //Relation: enables reproducibility and debugging by continuing from exact RNG state.
    bool restoreState(const std::string& state, bool fixed, std::uint32_t seed) {
        std::istringstream in(state);
        std::mt19937 restoredEngine;
        in >> restoredEngine;
        if (in.fail()) {
            return false;
        }

        fixedSeed = fixed;
        seedValue = seed;
        engine = restoredEngine;
        return true;
    }
    //Input: none
    //Output: true if initialized with fixed seed
    //Purpose: indicates whether randomness is reproducible
    //Relation: helps distinguish between testing vs. live randomness.
    bool usesFixedSeed() const {
        return fixedSeed;
    }
    //Input: none
    //Output: current seed value
    //Purpose: retrieves seed for logging or debugging
    //Relation: useful for tracking reproducibility.
    std::uint32_t seed() const {
        return seedValue;
    }

private:
    bool fixedSeed;
    std::uint32_t seedValue;
    std::mt19937 engine;
};
