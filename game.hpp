#pragma once

#include <ncurses.h>
#include <cstdint>
#include <ctime>
#include <string>
#include <vector>
#include <set>

#include "bank.hpp"
#include "board.hpp"
#include "cards.hpp"
#include "cpu_player.hpp"
#include "game_settings.h"
#include "history.hpp"
#include "player.hpp"
#include "random_service.hpp"
#include "rules.hpp"
#include "sabotage.h"
#include "tutorials.h"

class SaveManager;
struct SaveFileInfo;

class Game {
public:
    Game();
    explicit Game(std::uint32_t seed);
    ~Game();

    bool run();

private:
    friend class SaveManager;

    enum StartChoice {
        START_NEW_GAME,
        START_LOAD_GAME,
        START_QUIT_GAME
    };

    static const int MIN_W = 124;
    static const int MIN_H = 45;
    static const int TITLE_W = 124;
    static const int TITLE_H = 9;
    static const int BOARD_W = 82;
    static const int BOARD_H = 29;
    static const int INFO_W = 34;
    static const int INFO_H = 29;
    static const int MSG_W = 124;
    static const int MSG_H = 5;

    Board board;
    BoardViewMode boardViewMode;
    std::vector<Player> players;
    RuleSet rules;
    GameSettings settings;
    RandomService rng;
    CpuController cpu;
    DeckManager decks;
    Bank bank;
    SabotageManager sabotage;
    ActionHistory history;
    WINDOW* titleWin;
    WINDOW* boardWin;
    WINDOW* infoWin;
    WINDOW* msgWin;
    bool hasColor;
    int retiredCount;
    int currentPlayerIndex;
    int turnCounter;
    std::string gameId;
    std::string assignedSaveFilename;
    std::time_t createdTime;
    std::time_t lastSavedTime;
    bool autoAdvanceUi;
    bool setupInProgress;
    bool sabotageUnlockAnnounced;
    TutorialFlags tutorialFlags;
    std::vector<ActiveTrap> activeTraps;

    //Input: void
    //Output: bool (true if terminal size meets minimum dimensions, false if too small)
    //Purpose: checks if terminal size meets minimum dimensions for UI layout
    //Relation: called at game start and after terminal resize events
    bool isTerminalSizeValid() const;
    //Input: void
    //Output: bool (true if resized to minimum or larger, false on quit)
    //Purpose: loops until terminal reaches minimum size or player quits
    //Relation: called at game start and after terminal resize failure
    bool ensureMinSize() const;

    //Input: void
    //Output: void
    //Purpose: allocates ncurses windows for title, board, sidebar, message areas
    //Relation: called during setup and after terminal resize recovery
    void createWindows();
    //Input: void
    //Output: void
    //Purpose: deallocates and nullifies all ncurses windows safely
    //Relation: called during cleanup and before window recreation
    void destroyWindows();
    //Input: window pointer, y/x coordinates, text string
    //Output: void
    //Purpose: waits for ENTER/confirmation keypress with text display
    //Relation: used for simple confirmation popups
    void waitForEnter(WINDOW* w, int y, int x, const std::string& text) const;
    //Input: window pointer
    //Output: void
    //Purpose: applies background color and attributes to ncurses window
    //Relation: called during window creation and updates
    void applyWindowBg(WINDOW* w) const;
    //Input: entry string
    //Output: void
    //Purpose: adds a timestamped entry to the game's action history log
    //Relation: called after major game events for record keeping
    void addHistory(const std::string& entry);
    //Input: void
    //Output: void
    //Purpose: renders ncurses title bar with game branding
    //Relation: called during setup screens
    void drawSetupTitle() const;
    //Input: title string, value (int)
    //Output: void
    //Purpose: flashes spin result in message window with blinking colors
    //Relation: called after rollSpinner
    void flashSpinResult(const std::string& title, int value) const;
    //Input: value (int)
    //Output: void
    //Purpose: displays shiny big-number popup for rolled values
    //Relation: called after rollSpinner
    void showRollResultPopup(int value) const;
    //Input: action string, defaultName string
    //Output: string filename
    //Purpose: prompts player for save filename with defaults
    //Relation: called before saveCurrentGame
    bool promptForFilename(const std::string& action,
                           const std::string& defaultName,
                           std::string& filename);
    //Input: selected SaveFileInfo reference
    //Output: bool (true if selection made, false if cancelled)
    //Purpose: displays scrollable list of save files to load
    //Relation: called by loadSavedGame
    bool chooseSaveFileToLoad(SaveFileInfo& selected);
    //Input: void
    //Output: bool (true if game saved, false on error/cancel)
    //Purpose: saves current game state to file with game ID and timestamp
    //Relation: called by user save command and on save-and-quit
    bool saveCurrentGame();
    //Input: void
    //Output: bool (true if game loaded, false on error)
    //Purpose: loads previously saved game from file
    //Relation: called from title screen menu
    bool loadSavedGame();
    bool windowsValid; //member variable to track if windows are currently created, to avoid unnecessary create/destroy calls
    //Input: currentPlayer (int), msg string, detail string
    //Output: bool (true if layout recovered successfully)
    //Purpose: recreates windows and recovers game UI after terminal resize
    //Relation: called when terminal size changes during gameplay
    bool recoverTerminalLayout(int currentPlayer, const std::string& msg, const std::string& detail);

    //Input: void
    //Output: int (menu choice: 0-2 for new/load/quit)
    //Purpose: displays main title screen with mode selection
    //Relation: called in main game loop to start new games or load
    StartChoice showStartScreen();
    //Input: void
    //Output: bool (true if valid board display mode selected)
    //Purpose: prompts player to choose board view (1860, Follow Camera, or Classic Full)
    //Relation: called during game setup after choosing new game
    bool chooseBoardViewMode();
    //Input: void
    //Output: bool (true if custom rules configured successfully)
    //Purpose: allows player to modify game rules and settings
    //Relation: called during new game setup
    bool configureCustomRules();
    //Input: void
    //Output: void
    //Purpose: displays prerecorded game guide to all players
    //Relation: called from title screen menu
    void showTutorial();
    //Input: void
    //Output: void
    //Purpose: displays popup with game controls and shortcuts
    //Relation: called when player presses '?' or from help menu
    void showGuidePopup() const;
    //Input: void
    //Output: void
    //Purpose: resets all tutorial flags to show tutorials again on next playthrough
    //Relation: used for debug/cheat purposes
    void resetTutorialFlags();
    //Input: topic TutorialTopic
    //Output: void
    //Purpose: shows first-time tutorial popup for specific game mechanics
    //Relation: called before major game events for new players
    void maybeShowFirstTimeTutorial(TutorialTopic topic);
    //Input: playerIndex (int), payment PaymentResult
    //Output: void
    //Purpose: shows tutorial about loan mechanics when player takes first loan
    //Relation: called after loan payments
    void maybeShowLoanTutorial(int playerIndex, const PaymentResult& payment);
    //Input: playerIndex (int)
    //Output: bool (true if sabotage is unlocked for this player)
    //Purpose: checks if player has met requirements to use sabotage features
    //Relation: called before showing sabotage menu
    bool isSabotageUnlockedForPlayer(int playerIndex) const;
    //Input: playerIndex (int)
    //Output: void
    //Purpose: announces when a player first unlocks sabotage capabilities
    //Relation: called when sabotage becomes available
    void maybeShowSabotageUnlock(int playerIndex);
    //Input: void
    //Output: void
    //Purpose: displays popup with game controls and shortcuts
    //Relation: called when player presses '?' or from help menu
    void showControlsPopup() const;
    //Input: void
    //Output: void
    //Purpose: displays ranked scoreboard of all players with current standings
    //Relation: called when player presses 'S' or from menu
    void showScoreboardPopup() const;
    //Input: void
    //Output: void
    //Purpose: displays guide explaining different tile types and their effects
    //Relation: called when player presses 'T' or from help menu
    void showTileGuidePopup() const;
    int findPlayerIndex(const Player& player) const;
    bool isCpuPlayer(int playerIndex) const;
    //Input: playerIndex (int)
    //Output: void
    //Purpose: Temporarily displays a thinking animation for the CPU player with an action description
    //Relation: called during CPU turns to provide visual feedback and auto-advance delays
    void showCpuThinking(int playerIndex, const std::string& action) const;
    //Input: player reference
    //Output: int (effective salary value)
    //Purpose: calculates the player's actual salary after sabotage reductions are applied
    //Relation: used by PAYDAY tile effects and salary deductions
    int effectiveSalary(const Player& player) const;
    //Input: salary (int)
    //Output: int (adjusted salary value)
    //Purpose: applies game-wide salary multipliers or reductions
    //Relation: used by effectiveSalary and salary calculations
    int adjustedSalary(int salary) const;
    //Input: amount (int)
    //Output: int (reward amount)
    //Purpose: calculates reward payouts with potential multipliers
    //Relation: used by tile effects and bonuses
    int rewardAmount(int amount) const;
    //Input: amount (int)
    //Output: int (penalty amount)
    //Purpose: calculates penalty deductions with potential multipliers
    //Relation: used by tile effects and penalties
    int penaltyAmount(int amount) const;
    //Input: player reference
    //Output: void
    //Purpose: decrements all turn-limited status effects (penalties, reductions, cooldowns)
    //Relation: called at end of each player turn
    void decrementTurnStatuses(Player& player);
    //Input: playerIndex (int)
    //Output: bool (true if player has skipturn flag)
    //Purpose: checks if a player's next turn should be skipped and removes the flag
    //Relation: called at the start of each player turn to process skip-turn sabotage effects
    bool resolveSkipTurn(int playerIndex);
    //Input: playerIndex (int)
    //Output: void
    //Purpose: chances CPU to use sabotage during their turn based on difficulty and game state
    //Relation: called during CPU turns before movement spin
    void maybeCpuSabotage(int playerIndex);
    //Input: attacker index (int)
    //Output: bool (true if sabotage menu was handled and used)
    //Purpose: displays full sabotage menu with all options and handles player selection
    //Relation: called when player presses 'B' during their turn
    bool promptSabotageMenu(int attackerIndex);
    //Input: attacker index (int)
    //Output: int (target player index, or -1 if cancelled)
    //Purpose: displays list of valid sabotage targets and handles selection
    //Relation: used by most sabotage options
    int chooseSabotageTarget(int attackerIndex);
    //Input: attacker index (int)
    //Output: int (tile id to place trap on, or -1 if cancelled)
    //Purpose: prompts human to input tile id for trap placement
    //Relation: used by TRAP TILE sabotage option
    int chooseTrapTile(int attackerIndex);
    //Input: attacker index (int), target index (int), SabotageType type
    //Output: void
    //Purpose: resolves full sabotage action including rolls, damage, cooldowns, and UI
    //Relation: called by promptSabotageMenu and maybeCpuSabotage
    void executeSabotage(int attackerIndex, int targetIndex, SabotageType type);
    //Input: playerIndex (int), tileId (int), SabotageType type
    //Output: void
    //Purpose: adds an armed trap to a specific tile that triggers when other players land on it
    //Relation: used by TRAP TILE sabotage option
    void placeTrap(int attackerIndex, int tileId, SabotageType type);
    //Input: playerIndex (int)
    //Output: void
    //Purpose: checks the current tile for active traps and resolves their effects
    //Relation: called after player lands on a new tile
    void checkTrapTrigger(int playerIndex);
    //Input: void
    //Output: void
    //Purpose: initializes game rules from game settings and resets turn counter and game state
    //Relation: called before setupPlayers during new game setup
    void setupRules();
    //Input: void
    //Output: bool (true if players setup successfully)
    //Purpose: prompts for player count, types, names, and initializes player objects
    //Relation: called during new game setup
    bool setupPlayers();
    //Input: unavailable characters vector, name string, index int, type PlayerType
    //Output: char (selected character)
    //Purpose: displays character selection popup and handles choice
    //Relation: called during player setup for customization
    char showCharacterCustomisationPopup(const std::vector<char>& unavailable, const std::string& name, int index, PlayerType type);
    //Input: void
    //Output: void
    //Purpose: assigns investment cards to all players at game start
    //Relation: called during game setup after players are created
    void setupInvestments();
    //Input: currentPlayer (int)
    //Output: int (command keycode or selection)
    //Purpose: waits for human player keyboard input during their turn
    //Relation: called in main game loop before taking movement spin
    int waitForTurnCommand(int currentPlayer);
    //Input: currentPlayer (int), msg string, detail string
    //Output: void
    //Purpose: renders the full game screen with board, info, and message
    //Relation: called every turn and after UI events
    void renderGame(int currentPlayer, const std::string& msg, const std::string& detail) const;
    //Input: void
    //Output: void
    //Purpose: renders ncurses title bar
    //Relation: called during renderGame
    void renderHeader() const;
    //Input: title string, detail string
    //Output: int (rolled value)
    //Purpose: displays spinner animation and returns random roll result
    //Relation: used for movement and various game rolls
    int rollSpinner(const std::string& title, const std::string& detail);
    //Input: line1 string, line2 string
    //Output: void
    //Purpose: displays a simple two-line popup message
    //Relation: called throughout game for notifications
    void showInfoPopup(const std::string& line1, const std::string& line2) const;
    //Input: playerIndex, rolledValue, movementValue, startTile, endTile, startingCash, startingLoans, reason
    //Output: void
    //Purpose: displays detailed turn summary popup with movement and cash changes
    //Relation: called after movement spin
    void showTurnSummaryPopup(int playerIndex,
                              int rolledValue,
                              int movementValue,
                              int startTile,
                              int endTile,
                              int startingCash,
                              int startingLoans,
                              const std::string& reason) const;
    //Input: title string, lines vector, a char, b char
    //Output: int (selected option index or MENU_CANCELLED)
    //Purpose: displays a popup menu allowing player to choose between two or more options
    //Relation: used for optional binary/ternary choice scenarios throughout the game
    int showBranchPopup(const std::string& title,
                        const std::vector<std::string>& lines,
                        char a,
                        char b);
    //Input: title string, lines vector, a char, b char
    //Output: int (selected option index)
    //Purpose: displays a popup menu forcing player to choose between two or more options
    //Relation: used for required binary/ternary choice scenarios throughout the game
    int showRequiredBranchPopup(const std::string& title,
                                const std::vector<std::string>& lines,
                                char a,
                                char b);
    //Input: playerIndex (int)
    //Output: void
    //Purpose: plays black tile minigame, handles CPU simulation or human play, awards payout
    //Relation: called on BLACK tiles
    void playBlackTileMinigame(int playerIndex);
    //Input: currentPlayer (int)
    //Output: int (random opponent index or -1 if none available)
    //Purpose: selects a random non-retired opponent for duel/minigame challenges
    //Relation: used by duel minigame and forced minigame sabotage
    int chooseRandomOpponentIndex(int currentPlayer);
    //Input: player reference (const)
    //Output: int (simulated score 0-100)
    //Purpose: generates difficulty-appropriate minigame score for CPU players
    //Relation: used for CPU duel minigame outcomes
    int simulateDuelMinigameScore(const Player& player);
    //Input: playerIndex (int), minigameChoice (int)
    //Output: int (score 0-100)
    //Purpose: plays one of five minigames for human/CPU and returns normalized score
    //Relation: used by duel minigame and black tile minigame resolution
    int playDuelMinigameScore(int playerIndex, int minigameChoice);
    //Input: playerIndex (int), amountDelta reference, forcedOpponentIndex (int), pot (int)
    //Output: string (result message)
    //Purpose: orchestrates full duel minigame flow with random opponent selection, minigame play, and payout
    //Relation: called by action card and sabotage duel card effects
    std::string resolveDuelMinigameAction(int playerIndex,
                                          int& amountDelta,
                                          int forcedOpponentIndex = -1,
                                          int pot = 30000);
    //Input: playerIndex (int), tile reference
    //Output: int (money gained/lost from card)
    //Purpose: draws action card, resolves rolls/branches if needed, applies effect, shows popup
    //Relation: called on black tiles in 1860 mode and during action card plays
    int playActionCard(int playerIndex, const Tile& tile);
    //Input: playerIndex (int), tile reference
    //Output: void
    //Purpose: routes tile effects through appropriate resolver methods based on tile type
    //Relation: called after player lands on a new tile to trigger its game mechanic
    void applyTileEffect(int playerIndex, const Tile& tile);
    //Input: player reference, tileId (int)
    //Output: int (previous tile id)
    //Purpose: finds tile that leads to current tile (reverse navigation) for backward movement
    //Relation: used by action cards that move player backward
    int findPreviousTile(const Player& player, int tileId) const;
    //Input: fromTileId (current 1860 tile id)
    //Output: vector of adjacent legal 1860 tile ids
    //Purpose: finds one-step manual movement options without using classic next links
    //Relation: used by human and CPU 1860 movement
    std::vector<int> validAdjacent1860Tiles(int fromTileId) const;
    //Input: fromTileId and toTileId
    //Output: true if the step is legal for 1860 movement
    //Purpose: prevents invalid, blank, diagonal, and backward-loop 1860 movement
    //Relation: used by validAdjacent1860Tiles and movement validation
    bool isLegal1860Step(int fromTileId, int toTileId) const;
    //Input: currentPlayer index and remaining movement points
    //Output: next 1860 tile id or current tile id if no move is available
    //Purpose: chooses one CPU 1860 step with difficulty-aware retirement progress
    //Relation: used by moveCPUManually1860
    int chooseCPU1860NextStep(int currentPlayer, int remainingSteps) const;
    //Input: currentPlayer index and available movement points
    //Output: true if the human moved at least one 1860 step
    //Purpose: lets a human move tile-by-tile in 1860 mode using keyboard controls
    //Relation: used by takeMovementSpin and action-card movement in Mode1860
    bool moveHumanManually1860(int currentPlayer, int steps);
    //Input: currentPlayer index and available movement points
    //Output: true if the CPU moved at least one 1860 step
    //Purpose: moves CPU players one 1860 tile at a time toward Retirement
    //Relation: used by takeMovementSpin and action-card movement in Mode1860
    bool moveCPUManually1860(int currentPlayer, int steps);
    //Input: playerIndex (int), steps (int)
    //Output: string (movement result message)
    //Purpose: moves player by action card effect, handling special movement rules
    //Relation: called by action card effects
    std::string movePlayerByAction(int playerIndex, int steps);
    //Input: playerIndex (int), tile reference, ActionEffect structure, amountDelta reference
    //Output: string (result message)
    //Purpose: applies various action card effects (gain/lose money, move, minigame, etc.)
    //Relation: called by playActionCard to resolve drawn card mechanics
    std::string applyActionEffect(int playerIndex,
                                  const Tile& tile,
                                  const ActionEffect& effect,
                                  int& amountDelta);
    //Input: player reference, requiresDegree (bool)
    //Output: void
    //Purpose: displays a popup to choose between available career cards and assigns job/salary
    //Relation: called on CAREER and GRADUATION tiles
    void chooseCareer(Player& player, bool requiresDegree);
    //Input: player reference
    //Output: void
    //Purpose: prompts player to choose Family Path or Life Path and enables baby events if family chosen
    //Relation: called on SPLIT_FAMILY tiles
    void resolveFamilyStop(Player& player);
    //Input: player reference
    //Output: void
    //Purpose: allows player to pay $100K to upgrade their career after graduation via Night School
    //Relation: called on NIGHT_SCHOOL tiles when enabled
    void resolveNightSchool(Player& player);
    //Input: player reference
    //Output: void
    //Purpose: marks player as married and spins for gift money payout
    //Relation: called on MARRIAGE tiles
    void resolveMarriageStop(Player& player);
    //Input: player reference, tile reference
    //Output: void
    //Purpose: spins for 0-3 babies and adds them to player family on baby events
    //Relation: called on BABY tiles or triggered by family events
    void resolveBabyStop(Player& player, const Tile& tile);
    //Input: player reference
    //Output: void
    //Purpose: spins for a guaranteed small-to-medium cash payout on safe route
    //Relation: called on SAFE tiles after risk choice
    void resolveSafeRoute(Player& player);
    //Input: player reference
    //Output: void
    //Purpose: spins for either a large cash win or significant loss on risky route
    //Relation: called on RISKY tiles after risk choice
    void resolveRiskyRoute(Player& player);
    //Input: playerIndex (int)
    //Output: void
    //Purpose: handles end-of-game retirement destination choice and calculates retirement bonus
    //Relation: called on RETIREMENT tiles
    void resolveRetirement(int playerIndex);
    //Input: player reference
    //Output: void
    //Purpose: draws a house card and charges the player its cost, then stores house data
    //Relation: called on HOUSE tiles
    void buyHouse(Player& player);
    //Input: player reference
    //Output: void
    //Purpose: draws investment card from deck and assigns number/payout to player
    //Relation: called during setupInvestments for each player
    void assignInvestment(Player& player);
    //Input: spinnerValue (int)
    //Output: void
    //Purpose: checks all players' investment numbers against spin result and awards matching payouts
    //Relation: called after each movement spin to resolve investment bets
    void resolveInvestmentPayouts(int spinnerValue);
    //Input: spinnerValue (int)
    //Output: void
    //Purpose: awards Spin to Win token and bonus money if spinner lands on 10
    //Relation: called after each movement spin to check for lucky 10 result
    void maybeAwardSpinToWin(Player& player, int spinnerValue);
    //Input: player reference, reason string
    //Output: void
    //Purpose: adds a pet card from the deck to the player's collection with optional lore reason
    //Relation: called after marriage, house purchase, or as special bonuses
    void maybeAwardPetCard(Player& player, const std::string& reason);
    //Input: player reference, tile reference
    //Output: int (next tile id)
    //Purpose: determines which tile to move to next based on tile branching and player route choices
    //Relation: used during animated movement through the board
    int chooseNextTile(Player& player, const Tile& tile);
    //Input: playerIndex (int), steps (int)
    //Output: true if player moved at least one tile, false otherwise
    //Purpose: displays animated tile-by-tile movement and applies effects on stop spaces
    //Relation: called by takeMovementSpin for classic board mode
    bool animateMove(int currentPlayer, int steps);
    //Input: playerIndex (int), reason string
    //Output: void
    //Purpose: spins the movement wheel, applies penalties, then moves player and resolves tile effect
    //Relation: called once per player turn or by SPIN_AGAIN tiles
    void takeMovementSpin(int currentPlayer, const std::string& reason);
    //Input: void
    //Output: bool (true if all players have reached retirement)
    //Purpose: checks win condition for game loop
    //Relation: used in main game loop condition
    bool allPlayersRetired() const;
    //Input: void
    //Output: void
    //Purpose: displays house sale spins and calculates final scores for all players with breakdown UI
    //Relation: called after all players retire before showing winner
    void finalizeScoring();
    //Input: winnerIndex (int), winnerScore (int)
    //Output: void
    //Purpose: creates detailed game completion entry with rankings and appends to completed game history file
    //Relation: called after finalizeScoring to persist game results
    void appendCompletedGameHistoryEntry(int winnerIndex, int winnerScore);
    //Input: player reference (const)
    //Output: int (calculated net worth)
    //Purpose: sums cash, house value, card bonuses, and subtracts loan debt to get final score
    //Relation: used for ranking players and determining winner
    int calculateFinalWorth(const Player& player) const;

    //Input: tier (int)
    //Output: int (minimum reward for tier)
    //Purpose: returns the floor payout amount for action card outcomes by tier level
    //Relation: used for normalizing random action card results
    int minRewardForTier(int tier) const;
    //Input: tier (int)
    //Output: int (maximum reward for tier)
    //Purpose: returns the ceiling payout amount for action card outcomes by tier level
    //Relation: used for normalizing random action card results
    int maxRewardForTier(int tier) const;
    //Input: player reference
    //Output: void
    //Purpose: triggers a baby event for family path players
    //Relation: called randomly during turns
    void triggerBabyEvent(Player& player);
};
