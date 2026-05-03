#include "game.hpp"
#include "battleship.hpp"
#include "pong.hpp"
#include "hangman.hpp"
#include "memory.hpp"
#include "minesweeper.hpp"
#include "tile_display.h"
#include "timer_display.h"
#include "save_manager.hpp"
#include "spins.hpp"
#include "input_helpers.h"
#include "ui.h"
#include "ui_helpers.h"
#include "completed_history.h"
#include "turn_summary.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <sstream>

// Forward declarations
std::string appendLoanText(const std::string& base, const PaymentResult& payment);
std::string displayNameFromPath(const std::string& path);
std::string trimCopy(const std::string& text);
std::string displayNameFromPath(const std::string& path);

//Input: A string text.
//Output: A trimmed copy of the string (leading and trailing whitespace removed).
//Purpose: Cleans up user input or text before parsing.
//Relation: Used by parseStrictInt to ensure integers are parsed correctly without whitespace issues.
std::string trimCopy(const std::string& text) {
    std::size_t begin = 0;
    while (begin < text.size() && std::isspace(static_cast<unsigned char>(text[begin]))) {
        ++begin;
    }
    std::size_t end = text.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1]))) {
        --end;
    }
    return text.substr(begin, end - begin);
}

//Input: A string text, reference to an integer value.
//Output: Boolean (true if parsing succeeded, false otherwise). On success, value is set.
//Purpose: Strictly validates and converts a string into an integer.
//Relation: Relies on trimCopy. Used wherever integer input must be validated (e.g., user input, file parsing).
bool parseStrictInt(const std::string& text, int& value) {
    const std::string trimmed = trimCopy(text);
    if (trimmed.empty()) {
        return false;
    }
    std::size_t index = 0;
    if (trimmed[index] == '+' || trimmed[index] == '-') {
        ++index;
    }
    if (index >= trimmed.size()) {
        return false;
    }
    for (; index < trimmed.size(); ++index) {
        if (!std::isdigit(static_cast<unsigned char>(trimmed[index]))) {
            return false;
        }
    }
    char* end = nullptr;
    const long parsed = std::strtol(trimmed.c_str(), &end, 10);
    if (end == nullptr || *end != '\0' || parsed < -2147483647L - 1L || parsed > 2147483647L) {
        return false;
    }
    value = static_cast<int>(parsed);
    return true;
}

//Input: None.
//Output: A fixed string with instructions for turn commands.
//Purpose: Provides UI prompt text for players.
//Relation: Displayed in waitForTurnCommand and renderGame
std::string turnPromptText() {
    return "ENTER begin turn | B sabotage | TAB scores+map | G guide | K keys | S save | ESC menu";
}

//Input: A Tile object.
//Output: A string describing the effect of the tile.
//Purpose: Maps tile types to human-readable descriptions.
//Relation: Used in showTurnSummaryPopup and trap handling to explain tile effects.
std::string describeTileEffectText(const Tile& tile);

//Input: Constructor, optionally a seed.
//Output: A new Game object.
//Purpose: Initializes game state, rules, RNG, players, windows, and history.
//Relation: Core setup for the game engine.
Game::Game()
    : boardViewMode(BoardViewMode::FollowCamera),
      rules(makeNormalRules()),
      rng(),
      cpu(rng),
      decks(rules, rng),
      bank(rules),
      sabotage(bank, rng),
      history(8),
      titleWin(nullptr),
      boardWin(nullptr),
      infoWin(nullptr),
      msgWin(nullptr),
      hasColor(has_colors()),
      retiredCount(0),
      currentPlayerIndex(0),
      turnCounter(0),
      gameId(),
      assignedSaveFilename(),
      createdTime(0),
      lastSavedTime(0),
      autoAdvanceUi(false),
      setupInProgress(false),
      sabotageUnlockAnnounced(false),
      tutorialFlags(),
      activeTraps(), 
      windowsValid(false)
      {
}

//Input: Constructor, optionally a seed.
//Output: A new Game object.
//Purpose: Initializes game state, rules, RNG, players, windows, and history.
//Relation: Core setup for the game engine.
Game::Game(std::uint32_t seed)
    : boardViewMode(BoardViewMode::FollowCamera),
      rules(makeNormalRules()),
      rng(seed),
      cpu(rng),
      decks(rules, rng),
      bank(rules),
      sabotage(bank, rng),
      history(8),
      titleWin(nullptr),
      boardWin(nullptr),
      infoWin(nullptr),
      msgWin(nullptr),
      hasColor(has_colors()),
      retiredCount(0),
      currentPlayerIndex(0),
      turnCounter(0),
      gameId(),
      assignedSaveFilename(),
      createdTime(0),
      lastSavedTime(0),
      autoAdvanceUi(false),
      setupInProgress(false),
      sabotageUnlockAnnounced(false),
      tutorialFlags(),
      activeTraps(), 
      windowsValid(false)
      {
}

//Input: None.
//Output: None.
//Purpose: Cleans up resources (destroys windows).
//Relation: Ensures proper cleanup when a game ends.
Game::~Game() {
    destroyWindows();
}

//Input: A string entry.
//Output: None.
//Purpose: Adds a line to the game’s history log.
//Relation: Used throughout gameplay to record events.
void Game::addHistory(const std::string& entry) {
    history.add(entry);
}

//Input: None.
//Output: Boolean (true if saved successfully).
//Purpose: Saves the current game state to a file.
//Relation: Uses SaveManager, updates history, manages duplicate saves.
bool Game::saveCurrentGame() {
    SaveManager saveManager;
    std::string filename = assignedSaveFilename;
    if (filename.empty()) {
        if (!promptForFilename("Save", saveManager.defaultSaveFilename(), filename)) {
            return false;
        }
    }

    const std::string previousAssignedFilename = assignedSaveFilename;
    const std::time_t previousLastSavedTime = lastSavedTime;
    if (gameId.empty()) {
        gameId = saveManager.generateGameId();
    }
    if (createdTime == 0) {
        createdTime = std::time(0);
    }
    // The save file name can change over time, but the game id stays stable so
    // duplicate copies of the same run can be detected and archived.
    assignedSaveFilename = saveManager.normalizeFilename(filename);
    lastSavedTime = std::time(0);

    std::string error;
    if (!saveManager.saveGame(*this, assignedSaveFilename, error)) {
        assignedSaveFilename = previousAssignedFilename;
        lastSavedTime = previousLastSavedTime;
        showInfoPopup("Save failed", error);
        return false;
    }

    int archivedCount = 0;
    std::string archiveError;
    const bool duplicatesArchived =
        saveManager.archiveDuplicateSaves(gameId, assignedSaveFilename, archivedCount, archiveError);

    const std::string resolvedPath = saveManager.resolvePath(assignedSaveFilename);
    addHistory("Saved game to " + resolvedPath);
    if (archivedCount > 0) {
        addHistory("Archived " + std::to_string(archivedCount) + " duplicate save copies");
    }
    if (!duplicatesArchived) {
        addHistory("Warning: " + archiveError);
        showInfoPopup("Game saved", archiveError);
        return true;
    }
    if (archivedCount > 0) {
        showInfoPopup("Game saved", "Archived " + std::to_string(archivedCount) + " duplicate save copies.");
        return true;
    }

    showInfoPopup("Game saved", resolvedPath);
    return true;
}

//Input: None.
//Output: Boolean (true if loaded successfully).
//Purpose: Loads a saved game from disk.
//Relation: Uses SaveManager, updates history, handles duplicate saves.
bool Game::loadSavedGame() {
    SaveManager saveManager;
    while (true) {
        SaveFileInfo selected;
        if (!chooseSaveFileToLoad(selected)) {
            return false;
        }

        std::string error;
        if (!saveManager.loadGame(*this, selected.path, error)) {
            showInfoPopup("Load failed", error);
            continue;
        }

        addHistory("Loaded game from " + selected.filename);
        if (!assignedSaveFilename.empty() && selected.filename != assignedSaveFilename) {
            addHistory("Loaded duplicate copy for " + gameId);
            showInfoPopup("Game loaded", "Future saves overwrite " + saveManager.resolvePath(assignedSaveFilename));
            return true;
        }
        if (selected.duplicateGameId) {
            showInfoPopup("Game loaded", "Canonical save slot: " + saveManager.resolvePath(assignedSaveFilename));
            return true;
        }
        showInfoPopup("Game loaded", displayNameFromPath(selected.path));
        return true;
    }
}

//Input: Player index.
//Output: Boolean (true if player is CPU).
//Purpose: Checks if a given player is controlled by CPU.
//Relation: Used in sabotage, traps, and turn handling.
bool Game::isCpuPlayer(int playerIndex) const {
    return playerIndex >= 0 &&
           playerIndex < static_cast<int>(players.size()) &&
           players[static_cast<std::size_t>(playerIndex)].type == PlayerType::CPU;
}

//Input: A Player reference.
//Output: None.
//Purpose: Decreases temporary status effects (movement penalty, salary reduction, cooldowns).
//Relation: Called after turns or sabotage resolution
void Game::decrementTurnStatuses(Player& player) {
    if (player.movementPenaltyTurns > 0) {
        --player.movementPenaltyTurns;
        if (player.movementPenaltyTurns == 0) {
            player.movementPenaltyPercent = 0;
        }
    }
    if (player.salaryReductionTurns > 0) {
        --player.salaryReductionTurns;
        if (player.salaryReductionTurns == 0) {
            player.salaryReductionPercent = 0;
        }
    }
    if (player.sabotageCooldown > 0) {
        --player.sabotageCooldown;
    }
    if (player.itemDisableTurns > 0) {
        --player.itemDisableTurns;
    }
}

//Input: Player index.
//Output: Boolean (true if turn was skipped).
//Purpose: Handles sabotage effect where a player skips a turn.
//Relation: Updates history, UI, and player statuses.
bool Game::resolveSkipTurn(int playerIndex) {
    Player& player = players[static_cast<std::size_t>(playerIndex)];
    if (!player.skipNextTurn) {
        return false;
    }
    player.skipNextTurn = false;
    addHistory(player.name + " skipped a turn from sabotage");
    renderGame(playerIndex, player.name + " skips this turn", "Sabotage effect resolved.");
    const bool previousAutoAdvance = autoAdvanceUi;
    autoAdvanceUi = isCpuPlayer(playerIndex);
    showInfoPopup("Skip Turn", player.name + " loses this turn to sabotage.");
    autoAdvanceUi = previousAutoAdvance;
    decrementTurnStatuses(player);
    return true;
}


std::string describeTileEffectText(const Tile& tile) {
    switch (tile.kind) {
        case TILE_BLACK:
            return "Starts a minigame. Payout depends on performance.";
        case TILE_START:
            return "The journey begins.";
        case TILE_SPLIT_START:
            return "Choose College or Career before moving ahead.";
        case TILE_COLLEGE:
            return "College route tuition is paid before the route begins.";
        case TILE_CAREER:
            return "Choose a career card and set salary.";
        case TILE_GRADUATION:
            return "College players choose a degree career.";
        case TILE_MARRIAGE:
            return "Resolve marriage and gift spin.";
        case TILE_SPLIT_FAMILY:
            return "Choose Family path or Life path.";
        case TILE_FAMILY:
            return "Resolve the Family path choice.";
        case TILE_NIGHT_SCHOOL:
            return "Optional career upgrade if Night School is enabled.";
        case TILE_SPLIT_RISK:
            return "Choose Safe route or Risky route.";
        case TILE_SAFE:
            return "Collect a safe-route payout.";
        case TILE_RISKY:
            return "Spin for a high-risk reward or penalty.";
        case TILE_SPIN_AGAIN:
            return "Take another movement spin immediately.";
        case TILE_CAREER_2:
            return "Promotion increases salary.";
        case TILE_PAYDAY:
            return "Receive salary plus this space payout.";
        case TILE_BABY:
            return "Family event adds children based on the tile.";
        case TILE_RETIREMENT:
            return "Choose retirement destination and final bonus.";
        case TILE_HOUSE:
            return "Draw and buy a house.";
        case TILE_EMPTY:
        default:
            return "No special effect.";
    }
}

//Input: Attacker index, tile ID, sabotage type.
//Output: None.
//Purpose: Places a trap on the board.
//Relation: Updates history, charges attacker, adds trap to active list
void Game::placeTrap(int attackerIndex, int tileId, SabotageType type) {
    if (attackerIndex < 0 ||
        attackerIndex >= static_cast<int>(players.size()) ||
        tileId < 0 ||
        tileId >= (boardViewMode == BoardViewMode::Mode1860 ? board.tileCount() : TILE_COUNT) ||
        (boardViewMode == BoardViewMode::Mode1860 && !board.isMode1860WalkableTile(tileId))) {
        return;
    }
    Player& attacker = players[static_cast<std::size_t>(attackerIndex)];
    const SabotageCard card = sabotageCardByName("Trap Tile");
    PaymentResult payment = bank.charge(attacker, card.costToUse);
    maybeShowLoanTutorial(attackerIndex, payment);

    ActiveTrap trap;
    trap.tileId = tileId;
    trap.ownerIndex = attackerIndex;
    trap.effectType = type;
    trap.strengthLevel = 2;
    trap.armed = true;
    activeTraps.push_back(trap);

    addHistory(attacker.name + " placed a " + sabotageTypeName(type) + " trap on tile " +
               std::to_string(tileId));
    std::string detail = "Trap armed on tile " + std::to_string(tileId) + ".";
    if (payment.loansTaken > 0) {
        detail += " Automatic loans: " + std::to_string(payment.loansTaken) + ".";
    }
    showInfoPopup("Trap Tile", detail);
}

//Input: Attacker index, target index, sabotage type.
//Output: None.
//Purpose: Executes sabotage action (lawsuit, minigame, direct sabotage).
//Relation: Uses RNG, shields, bank, tutorials, updates history.
void Game::executeSabotage(int attackerIndex, int targetIndex, SabotageType type) {
    if (attackerIndex < 0 ||
        targetIndex < 0 ||
        attackerIndex >= static_cast<int>(players.size()) ||
        targetIndex >= static_cast<int>(players.size()) ||
        attackerIndex == targetIndex) {
        return;
    }
    Player& attacker = players[static_cast<std::size_t>(attackerIndex)];
    Player& target = players[static_cast<std::size_t>(targetIndex)];
    if (attacker.sabotageCooldown > 0) {
        showInfoPopup("Sabotage cooldown",
                      attacker.name + " must wait " + std::to_string(attacker.sabotageCooldown) + " more turn(s).");
        return;
    }

    const SabotageCard card = type == SabotageType::MoneyLoss
        ? sabotageCardByName("Lawsuit")
        : sabotageCardForType(type);
    PaymentResult cost = bank.charge(attacker, card.costToUse);
    maybeShowLoanTutorial(attackerIndex, cost);
    SabotageResult result;
    const bool humanDrivenSabotage = !isCpuPlayer(attackerIndex);
    if (type == SabotageType::ForceMinigame && humanDrivenSabotage) {
        std::string shieldText;
        if (sabotage.consumeShield(target, shieldText)) {
            result.blocked = true;
            result.summary = shieldText;
        } else {
            const int duelRoll = rollSpinner(card.name, "Spin to see if the duel challenge lands");
            result.attempted = true;
            result.roll = duelRoll;
            result.critical = duelRoll >= 8;
            if (duelRoll <= 3) {
                result.summary = card.name + " failed on roll " + std::to_string(duelRoll) + ".";
            } else {
                result.summary = resolveDuelMinigameAction(attackerIndex, result.amount, targetIndex, 40000);
                result.success = result.amount >= 0;
            }
        }
    } else if (card.name == "Lawsuit" && humanDrivenSabotage) {
        std::string shieldText;
        if (sabotage.consumeShield(target, shieldText)) {
            result.blocked = true;
            result.summary = shieldText;
        } else {
            const int attackerRoll = rollSpinner("Lawsuit", attacker.name + " spins the case");
            const int targetRoll = isCpuPlayer(targetIndex)
                ? rng.roll10()
                : rollSpinner("Lawsuit Defense", target.name + " spins the defense");
            result = sabotage.resolveLawsuit(attacker, target, attackerRoll, targetRoll);
        }
    } else if (card.requiresDiceRoll && humanDrivenSabotage) {
        const int sabotageRoll = rollSpinner(card.name, "Spin to see whether the sabotage lands");
        result = sabotage.applyDirectSabotage(card,
                                              attacker,
                                              target,
                                              players,
                                              attackerIndex,
                                              targetIndex,
                                              sabotageRoll);
    } else {
        result = sabotage.applyDirectSabotage(card,
                                              attacker,
                                              target,
                                              players,
                                              attackerIndex,
                                              targetIndex);
    }
    if (result.success && type == SabotageType::DebtIncrease && !isCpuPlayer(targetIndex)) {
        maybeShowFirstTimeTutorial(TutorialTopic::ManualLoan);
    }
    if (result.blocked && !isCpuPlayer(targetIndex)) {
        maybeShowFirstTimeTutorial(TutorialTopic::Shield);
    }
    if (result.summary.find("Insurance reduced") != std::string::npos && !isCpuPlayer(targetIndex)) {
        maybeShowFirstTimeTutorial(TutorialTopic::Insurance);
    }
    if (type == SabotageType::PositionSwap && result.success) {
        const int cooldown = result.critical ? 3 : 4;
        attacker.sabotageCooldown = std::max(attacker.sabotageCooldown, cooldown + 1);
    } else if (result.success || result.blocked) {
        attacker.sabotageCooldown = std::max(attacker.sabotageCooldown, 2);
    }

    std::string detail = result.summary;
    if (cost.loansTaken > 0) {
        detail += " Cost automatic loans: " + std::to_string(cost.loansTaken) + ".";
    }
    addHistory(attacker.name + " used " + card.name + " on " + target.name);
    addHistory(detail);
    showInfoPopup(card.name, detail);
}

//Input: Player index.
//Output: None.
//Purpose: Checks if a player triggered a trap and applies effects.
//Relation: Moves player back, triggers minigames, updates history.
void Game::checkTrapTrigger(int playerIndex) {
    Player& player = players[static_cast<std::size_t>(playerIndex)];
    for (size_t i = 0; i < activeTraps.size(); ++i) {
        ActiveTrap& trap = activeTraps[i];
        if (!trap.armed || trap.tileId != player.tile || trap.ownerIndex == playerIndex) {
            continue;
        }

        trap.armed = false;
        SabotageResult result = isCpuPlayer(playerIndex)
            ? sabotage.triggerTrap(trap, player)
            : sabotage.triggerTrap(trap,
                                   player,
                                   rollSpinner("Trap Trigger", "Spin to see whether the trap catches you"));
        addHistory(player.name + " triggered a trap on tile " + std::to_string(trap.tileId));
        addHistory(result.summary);
        showInfoPopup("Trap triggered", result.summary);

        if (result.success && trap.effectType == SabotageType::MovementPenalty) {
            const int stepsBack = result.critical ? 3 : 2;
            if (boardViewMode == BoardViewMode::Mode1860) {
                for (int step = 0; step < stepsBack && board.isMode1860WalkableTile(player.tile); ++step) {
                    const Tile& current = board.tileAt(player.tile);
                    const int downTile = board.mode1860TileIdAt(current.mode1860Y + 1, current.mode1860X);
                    const int leftTile = board.mode1860TileIdAt(current.mode1860Y, current.mode1860X - 1);
                    int nextTile = player.tile;
                    if (board.isMode1860WalkableTile(downTile)) {
                        nextTile = downTile;
                    } else if (board.isMode1860WalkableTile(leftTile)) {
                        nextTile = leftTile;
                    }
                    if (nextTile == player.tile) {
                        break;
                    }
                    player.tile = nextTile;
                }
            } else {
                for (int step = 0; step < stepsBack; ++step) {
                    player.tile = findPreviousTile(player, player.tile);
                }
            }
            renderGame(playerIndex,
                       player.name + " was pushed backward by a trap",
                       "Moved back " + std::to_string(stepsBack) + " spaces.");
            napms(450);
        } else if (result.success && trap.effectType == SabotageType::ForceMinigame) {
            playBlackTileMinigame(playerIndex);
        }
        return;
    }
}

//Input: None.
//Output: None.
//Purpose: Configures rules and resets game state.
//Relation: Called at game start or reset.
void Game::setupRules() {
    SaveManager saveManager;
    validateGameSettings(settings);
    rules = makeNormalRules();
    applyGameSettingsToRules(settings, rules);
    decks.reset(rules);
    bank.configure(rules);
    retiredCount = 0;
    currentPlayerIndex = 0;
    turnCounter = 0;
    gameId = saveManager.generateGameId();
    assignedSaveFilename.clear();
    createdTime = std::time(0);
    lastSavedTime = 0;
    activeTraps.clear();
    history.clear();
    addHistory("Mode: " + rules.editionName);
}

//Input: Current player index.
//Output: Key code (int).
//Purpose: Waits for player input during their turn.
//Relation: Handles UI commands (save, sabotage, guide, controls).
int Game::waitForTurnCommand(int currentPlayer) {
    if (isCpuPlayer(currentPlayer)) {
        showCpuThinking(currentPlayer, "CPU is planning its turn...");
        return '\n';
    }

    while (true) {
        int ch = wgetch(infoWin);

        if (ch == KEY_RESIZE) {
            if (!recoverTerminalLayout(currentPlayer, 
                                       players[currentPlayer].name + "'s turn", 
                                       turnPromptText())) {
                return 27;  // Quit if can't recover
            }

            if (!ensureMinSize()) {
                return 27;
            }
            destroyWindows();
            createWindows();
            renderGame(currentPlayer,
                       players[currentPlayer].name + "'s turn",
                       turnPromptText());
            continue;
        }
        if (isCancelKey(ch)) return 27;
        if (ch == 's' || ch == 'S') return ch;
        if (ch == 'b' || ch == 'B') return ch;
        if (ch == '\t') {
            showScoreboardPopup();
            renderGame(currentPlayer,
                       players[currentPlayer].name + "'s turn",
                       turnPromptText());
            continue;
        }
        if (ch == 'g' || ch == 'G') {
            showGuidePopup();
            renderGame(currentPlayer,
                       players[currentPlayer].name + "'s turn",
                       turnPromptText());
            continue;
        }
        if (ch == 'k' || ch == 'K' || ch == '?') {
            showControlsPopup();
            renderGame(currentPlayer, players[currentPlayer].name + "'s turn", turnPromptText());
            continue;
        }
        if (isConfirmKey(ch)) {
            return ch;
        }
    }
}

//Input: None.
//Output: None.
//Purpose: Draws the title banner.
//Relation: Called in renderGame
void Game::renderHeader() const {
    if (!titleWin) {
        return;
    }
    draw_title_banner_ui(titleWin);
}

//Input: Current player index, message, detail.
//Output: None.
//Purpose: Renders the board, sidebar, and message UI.
//Relation: Central rendering function.
void Game::renderGame(int currentPlayer, const std::string& msg, const std::string& detail) const {
    renderHeader();
    draw_board_ui(boardWin, board, players, currentPlayer, players[currentPlayer].tile, boardViewMode);
    draw_sidebar_ui(infoWin, board, players, currentPlayer, history.recent(), rules);
    draw_message_ui(msgWin, msg, detail);
}

//Input: Tier number.
//Output: Minimum/maximum reward.
//Purpose: Defines reward ranges by tier.
//Relation: Used in payouts.
int Game::minRewardForTier(int tier) const {
    if (tier == 2) return 3000;
    if (tier >= 3) return 5000;
    return 1000;
}

//Input: Tier number.
//Output: Minimum/maximum reward.
//Purpose: Defines reward ranges by tier.
//Relation: Used in payouts.
int Game::maxRewardForTier(int tier) const {
    if (tier == 2) return 5000;
    if (tier >= 3) return 10000;
    return 2000;
}

//Input: Two strings.
//Output: None.
//Purpose: Displays a popup message.
//Relation: Used throughout gameplay for feedback
void Game::showInfoPopup(const std::string& line1, const std::string& line2) const {
    if (titleWin) { touchwin(titleWin); wrefresh(titleWin); }
    if (boardWin) { touchwin(boardWin); wrefresh(boardWin); }
    if (infoWin) { touchwin(infoWin); wrefresh(infoWin); }
    if (msgWin) { touchwin(msgWin); wrefresh(msgWin); }
    showPopupMessage(line1, line2, hasColor, autoAdvanceUi);
    if (titleWin) { touchwin(titleWin); wrefresh(titleWin); }
    if (boardWin) { touchwin(boardWin); wrefresh(boardWin); }
    if (infoWin) { touchwin(infoWin); wrefresh(infoWin); }
    if (msgWin) { touchwin(msgWin); wrefresh(msgWin); }
}

//Input: Player index, rolled value, movement, start/end tiles, starting cash/loans, reason.
//Output: None.
//Purpose: Shows a detailed summary of a player’s turn.
//Relation: Uses describeTileEffectText, updates UI, creates TurnSummary
void Game::showTurnSummaryPopup(int playerIndex,
                                int rolledValue,
                                int movementValue,
                                int startTile,
                                int endTile,
                                int startingCash,
                                int startingLoans,
                                const std::string& reason) const {
    if (playerIndex < 0 || playerIndex >= static_cast<int>(players.size())) {
        return;
    }

    const Player& player = players[static_cast<std::size_t>(playerIndex)];
    const Tile& start = board.tileAt(startTile);
    const Tile& end = board.tileAt(endTile);
    std::vector<std::string> lines;
    lines.push_back(player.name + "'s Turn");
    lines.push_back("Player Type: " + playerTypeLabel(player.type) +
                    (player.type == PlayerType::CPU
                         ? " (" + cpuDifficultyLabel(player.cpuDifficulty) + ")"
                         : ""));
    lines.push_back("Start Money: $" + std::to_string(startingCash) +
                    " | Loans: " + std::to_string(startingLoans));
    lines.push_back("Start Position: Space " + std::to_string(startTile) +
                    " - " + getTileDisplayName(start));
    lines.push_back(reason + ": rolled " + std::to_string(rolledValue) +
                    (movementValue != rolledValue
                         ? " | movement after effects " + std::to_string(movementValue)
                         : ""));
    lines.push_back(player.name + " moved from Space " + std::to_string(startTile) +
                    " to Space " + std::to_string(endTile) + ".");
    lines.push_back("Landed On: " + getTileDisplayName(end));
    lines.push_back("Effect: " + describeTileEffectText(end));
    const int cashDelta = player.cash - startingCash;
    const int loanDelta = player.loans - startingLoans;
    std::string delta = "Money Change: ";
    if (cashDelta >= 0) {
        delta += "+$" + std::to_string(cashDelta);
    } else {
        delta += "-$" + std::to_string(-cashDelta);
    }
    delta += " | Loan Change: ";
    delta += loanDelta >= 0 ? "+" + std::to_string(loanDelta) : std::to_string(loanDelta);
    lines.push_back(delta);
    lines.push_back("End Money: $" + std::to_string(player.cash) +
                    " | Loans: " + std::to_string(player.loans));
    TurnSummary summary;
    summary.playerName = player.name;
    summary.turnNumber = player.turnsTaken + 1;
    summary.moneyChange = player.cash - startingCash;
    summary.loanChange = player.loans - startingLoans;
    summary.babyChange = 0;
    summary.petChange = 0;
    summary.investmentChange = 0;
    summary.shieldChange = 0;
    summary.insuranceChange = 0;
    summary.gotMarried = player.married && board.tileAt(endTile).kind == TILE_MARRIAGE;
    summary.jobChanged = false;
    summary.houseChanged = false;
    summary.importantEvents.push_back(reason + ": rolled " + std::to_string(rolledValue) +
                                      (movementValue != rolledValue ? " and moved " + std::to_string(movementValue) : ""));
    summary.importantEvents.push_back("Moved from Space " + std::to_string(startTile) +
                                      " to Space " + std::to_string(endTile) + ".");
    summary.importantEvents.push_back("Landed on " + getTileDisplayName(end) + ".");
    summary.importantEvents.push_back(describeTileEffectText(end));
    if (titleWin) { touchwin(titleWin); wrefresh(titleWin); }
    if (boardWin) { touchwin(boardWin); wrefresh(boardWin); }
    if (infoWin) { touchwin(infoWin); wrefresh(infoWin); }
    if (msgWin) { touchwin(msgWin); wrefresh(msgWin); }
    showTurnSummaryReport(summary, hasColor);
    if (titleWin) touchwin(titleWin);
    if (boardWin) touchwin(boardWin);
    if (infoWin) touchwin(infoWin);
    if (msgWin) touchwin(msgWin);
}

//Input: Title and detail strings.
//Output: Integer (spin result 1–10).
//Purpose: Simulates spinner roll with UI animation.
//Relation: Used in sabotage, traps, and minigames
int Game::rollSpinner(const std::string& title, const std::string& detail) {
    int msgH = 0;
    int msgW = 0;
    getmaxyx(msgWin, msgH, msgW);
    (void)msgH;
    const int contentW = std::max(1, msgW - 4);
    werase(msgWin);
    drawBoxSafe(msgWin);
    mvwprintw(msgWin, 1, 2, "%s", clipUiText(title, static_cast<std::size_t>(contentW)).c_str());
    mvwprintw(msgWin, 2, 2, "%s", clipUiText(detail, static_cast<std::size_t>(contentW)).c_str());
    wrefresh(msgWin);

    if (autoAdvanceUi) {
        int value = 1;
        for (int flash = 0; flash < 6; ++flash) {
            value = rng.roll10();
            werase(msgWin);
            drawBoxSafe(msgWin);
            mvwprintw(msgWin, 1, 2, "%s", clipUiText(title, static_cast<std::size_t>(contentW)).c_str());
            mvwprintw(msgWin, 2, 2, "CPU rolling: %d", value);
            wrefresh(msgWin);
            napms(90);
        }
        flashSpinResult(title, value);
        showRollResultPopup(value);
        addHistory("CPU auto-spin result: " + std::to_string(value));
        return value;
    }

    int ch;
    do {
        ch = wgetch(msgWin);
    } while (!isConfirmKey(ch, true));

    nodelay(msgWin, TRUE);
    auto lastSpace = std::chrono::steady_clock::now();
    int value = 1;
    while (true) {
        value = rng.roll10();
        werase(msgWin);
        drawBoxSafe(msgWin);
        mvwprintw(msgWin, 1, 2, "%s", clipUiText(title, static_cast<std::size_t>(contentW)).c_str());
        mvwprintw(msgWin, 2, 2, "%s",
                  clipUiText("Rolling: " + std::to_string(value) + "  Release SPACE to stop",
                             static_cast<std::size_t>(contentW)).c_str());
        wrefresh(msgWin);
        napms(80);

        ch = wgetch(msgWin);
        if (ch == ' ') {
            lastSpace = std::chrono::steady_clock::now();
        } else if (ch != ERR) {
            break;
        }
        long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - lastSpace).count();
        if (elapsed > 240) break;
    }
    nodelay(msgWin, FALSE);

    flashSpinResult(title, value);
    showRollResultPopup(value);
    werase(msgWin);
    drawBoxSafe(msgWin);
    const std::string settle = "The wheel settles on " + std::to_string(value) + ". Press ENTER to continue the story.";
    const std::string clippedTitle = clipUiText(title, static_cast<std::size_t>(contentW));
    const std::string clippedSettle = clipUiText(settle, static_cast<std::size_t>(contentW));
    mvwprintw(msgWin, 1, 2, "%s", clippedTitle.c_str());
    mvwprintw(msgWin, 2, 2, "%s", clippedSettle.c_str());
    wrefresh(msgWin);
    waitForEnter(msgWin, 2, 2, "");
    return value;
}

//Input: Current player index.
//Output: Opponent index or -1.
//Purpose: Selects a random opponent who is not retired.
//Relation: Used in sabotage targeting
int Game::chooseRandomOpponentIndex(int currentPlayer) {
    std::vector<int> candidates;
    for (size_t i = 0; i < players.size(); ++i) {
        if (static_cast<int>(i) != currentPlayer && !players[i].retired) {
            candidates.push_back(static_cast<int>(i));
        }
    }
    if (candidates.empty()) {
        return -1;
    }
    return candidates[static_cast<std::size_t>(rng.uniformInt(0, static_cast<int>(candidates.size()) - 1))];
}

//Input: Player reference.
//Output: Integer score.
//Purpose: Simulates CPU minigame performance.
//Relation: Used in duels when CPU plays.
int Game::simulateDuelMinigameScore(const Player& player) {
    if (player.type != PlayerType::CPU) {
        return rng.uniformInt(40, 85);
    }

    switch (player.cpuDifficulty) {
        case CpuDifficulty::Easy:
            return rng.uniformInt(20, 55);
        case CpuDifficulty::Hard:
            return rng.uniformInt(65, 95);
        case CpuDifficulty::Normal:
        default:
            return rng.uniformInt(45, 75);
    }
}

//Input: Player index, minigame choice.
//Output: Integer score.
//Purpose: Plays or simulates duel minigame.
//Relation: Calls specific minigame functions (Pong, Battleship, Hangman, etc.)
int Game::playDuelMinigameScore(int playerIndex, int minigameChoice) {
    Player& player = players[static_cast<std::size_t>(playerIndex)];
    static const char* names[] = {
        "Pong",
        "Battleship",
        "Hangman",
        "Memory Match",
        "Minesweeper"
    };

    if (isCpuPlayer(playerIndex)) {
        const int score = simulateDuelMinigameScore(player);
        showPopupMessage(player.name + " CPU Minigame Result",
                         cpuDifficultyLabel(player.cpuDifficulty) + " CPU scored " +
                         std::to_string(score) + "/100 in " + names[minigameChoice] + ".",
                         hasColor,
                         autoAdvanceUi);
        return score;
    }

    int score = 0;
    if (minigameChoice == 0) {
        const PongMinigameResult result = playPongMinigame(player.name, hasColor);
        score = result.abandoned ? 0 : std::min(100, result.playerScore * 10);
    } else if (minigameChoice == 1) {
        const BattleshipMinigameResult result = playBattleshipMinigame(player.name, hasColor);
        score = result.abandoned ? 0 : std::min(100, result.shipsDestroyed * 10);
    } else if (minigameChoice == 2) {
        const HangmanResult result = playHangmanMinigame(player.name, hasColor);
        score = result.abandoned ? 0 : std::min(100, result.lettersGuessed * 8);
    } else if (minigameChoice == 3) {
        const MemoryMatchResult result = playMemoryMatchMinigame(player.name, hasColor);
        score = result.abandoned ? 0 : std::min(100, result.pairsMatched * 12 + (result.won ? 4 : 0));
    } else {
        const MinesweeperResult result = playMinesweeperMinigame(player.name, hasColor);
        score = result.abandoned ? 0 : std::min(100, result.safeTilesRevealed * 6);
    }

    if (titleWin) touchwin(titleWin);
    if (boardWin) touchwin(boardWin);
    if (infoWin) touchwin(infoWin);
    if (msgWin) touchwin(msgWin);

    showInfoPopup(player.name + " duel score",
                  std::string(names[minigameChoice]) + " score: " + std::to_string(score) + "/100.");
    return score;
}

//Input:playerIndex: Index of the player initiating the duel.
//      amountDelta: Reference to an integer that will store the net money change (positive if player wins, negative if loses).
//      forcedOpponentIndex: Index of a specific opponent, or -1 to select randomly.
//pot: The wagered amount of money for the duel.
//Output: Returns a string summarizing the duel result (who won, scores, payouts, loans).
//Purpose:Executes a duel minigame between a player and an opponent.
//        Randomly selects a minigame (Pong, Battleship, Hangman, Memory Match, Minesweeper).
//        Determines scores, applies payouts, handles loans, and updates history.
//Relation:Calls chooseRandomOpponentIndex if no opponent is forced.
//        Uses playPongDuelMinigame or playDuelMinigameScore depending on minigame.
//        Interacts with bank for charging/crediting money.
//        Updates game history and shows UI popups.
//        Closely tied to sabotage mechanics (executeSabotage) when a duel minigame is triggered.
std::string Game::resolveDuelMinigameAction(int playerIndex,
                                            int& amountDelta,
                                            int forcedOpponentIndex,
                                            int pot) {
    Player& player = players[static_cast<std::size_t>(playerIndex)];
    const int opponentIndex = forcedOpponentIndex >= 0
        ? forcedOpponentIndex
        : chooseRandomOpponentIndex(playerIndex);
    if (opponentIndex < 0) {
        amountDelta = 0;
        return "No valid opponent is available.";
    }

    Player& opponent = players[static_cast<std::size_t>(opponentIndex)];
    const int minigameChoice = rng.uniformInt(0, 4);
    static const char* names[] = {
        "Pong",
        "Battleship",
        "Hangman",
        "Memory Match",
        "Minesweeper"
    };

    std::vector<std::string> lines;
    lines.push_back(player.name + " used a Duel Minigame Card!");
    lines.push_back("Random opponent draw begins...");
    lines.push_back("Opponent selected: " + opponent.name);
    lines.push_back(player.name + " will play against " + opponent.name + ".");
    if (opponent.type == PlayerType::CPU) {
        lines.push_back("Opponent CPU difficulty: " + cpuDifficultyLabel(opponent.cpuDifficulty));
    } else {
        lines.push_back("Opponent is human and will play normally.");
    }
    lines.push_back("Minigame selected: " + std::string(names[minigameChoice]));
    showPopupMessage("Duel Minigame Card", lines, hasColor, autoAdvanceUi);

    int playerScore = 0;
    int opponentScore = 0;
    if (minigameChoice == 0) {
        const PongDuelResult duelResult = playPongDuelMinigame(player.name,
                                                               opponent.name,
                                                               hasColor,
                                                               isCpuPlayer(opponentIndex));
        if (duelResult.abandoned) {
            amountDelta = 0;
            return "The duel ended early with no payout.";
        }
        playerScore = duelResult.winnerSide == 0 ? 100 : 0;
        opponentScore = duelResult.winnerSide == 1 ? 100 : 0;
    } else {
        playerScore = playDuelMinigameScore(playerIndex, minigameChoice);
        opponentScore = playDuelMinigameScore(opponentIndex, minigameChoice);
    }

    std::ostringstream result;
    result << names[minigameChoice] << " duel score "
           << player.name << " " << playerScore
           << " - " << opponent.name << " " << opponentScore << ". ";

    if (playerScore >= opponentScore) {
        PaymentResult payment = bank.charge(opponent, pot);
        maybeShowLoanTutorial(opponentIndex, payment);
        bank.credit(player, pot);
        amountDelta = pot;
        result << player.name << " wins $" << pot << ".";
        if (payment.loansTaken > 0) {
            result << " " << opponent.name << " automatic loans: " << payment.loansTaken << ".";
        }
    } else {
        PaymentResult payment = bank.charge(player, pot);
        maybeShowLoanTutorial(playerIndex, payment);
        bank.credit(opponent, pot);
        amountDelta = -pot;
        result << opponent.name << " wins $" << pot << ".";
        if (payment.loansTaken > 0) {
            result << " Automatic loans: " << payment.loansTaken << ".";
        }
    }

    addHistory(player.name + " dueled " + opponent.name + " in " + names[minigameChoice]);
    addHistory(result.str());
    showPopupMessage("Duel Result", result.str(), hasColor, autoAdvanceUi);
    return result.str();
}

//Input:player: Reference to the player.
//      tileId: Current tile ID.
//Output:Returns the ID of the previous tile (or the same tile if none found).
//Purpose: Finds the tile that leads into the given tile, effectively moving backwards on the board.
//         Handles special branching cases (college/career, family/life, safe/risky paths).
//Relation: Used in trap resolution (checkTrapTrigger) when a player is forced backward.
//          Relies on board structure (tile.next, tile.altNext) to determine valid predecessors.
int Game::findPreviousTile(const Player& player, int tileId) const {
    std::vector<int> candidates;
    for (int i = 0; i < TILE_COUNT; ++i) {
        const Tile& tile = board.tileAt(i);
        if (tile.next == tileId || tile.altNext == tileId) {
            candidates.push_back(i);
        }
    }

    if (candidates.empty()) {
        return tileId;
    }
    if (candidates.size() == 1) {
        return candidates[0];
    }

    if (tileId == 38) {
        return player.startChoice == 0 ? 24 : 37;
    }
    if (tileId == 79) {
        return player.familyChoice == 0 ? 68 : 78;
    }
    if (tileId == 86) {
        return player.riskChoice == 0 ? 82 : 85;
    }

    return candidates.back();
}

//Input: Board reference and 1860 tile id
//Output: integer progress score where larger means closer to top-right
//Purpose: gives 1860 movement a simple forward direction without classic next links
//Relation: used by isLegal1860Step and CPU movement scoring
static int mode1860ProgressScore(const Board& board, int tileId) {
    if (!board.isMode1860WalkableTile(tileId)) {
        return -1000000;
    }
    const Tile& tile = board.tileAt(tileId);
    return (board.mode1860Rows() - 1 - tile.mode1860Y) + tile.mode1860X;
}

//Input: Board reference and 1860 tile id
//Output: Manhattan distance to the 1860 retirement tile
//Purpose: lets CPU movement prefer paths that visibly approach Retirement
//Relation: used by chooseCPU1860NextStep
static int mode1860DistanceToRetirement(const Board& board, int tileId) {
    if (!board.isMode1860WalkableTile(tileId)) {
        return 1000000;
    }
    const Tile& tile = board.tileAt(tileId);
    const Tile& retirement = board.tileAt(board.mode1860RetirementTileId());
    return std::abs(tile.mode1860Y - retirement.mode1860Y) +
           std::abs(tile.mode1860X - retirement.mode1860X);
}

//Input: fromTileId and toTileId
//Output: true if toTileId is a legal one-step 1860 move
//Purpose: blocks invalid, diagonal, and backward-progress loop movement in 1860 mode
//Relation: used by validAdjacent1860Tiles and manual movement
bool Game::isLegal1860Step(int fromTileId, int toTileId) const {
    if (!board.isMode1860WalkableTile(fromTileId) ||
        !board.isMode1860WalkableTile(toTileId) ||
        fromTileId == toTileId) {
        return false;
    }

    const Tile& from = board.tileAt(fromTileId);
    const Tile& to = board.tileAt(toTileId);
    const int distance = std::abs(from.mode1860Y - to.mode1860Y) +
                         std::abs(from.mode1860X - to.mode1860X);
    if (distance != 1) {
        return false;
    }

    return toTileId == board.mode1860RetirementTileId() ||
           mode1860ProgressScore(board, toTileId) > mode1860ProgressScore(board, fromTileId);
}

//Input: fromTileId (current 1860 tile id)
//Output: legal adjacent 1860 tile ids
//Purpose: finds one-step movement options for manual 1860 movement
//Relation: used by human and CPU 1860 movement
std::vector<int> Game::validAdjacent1860Tiles(int fromTileId) const {
    std::vector<int> tiles;
    if (!board.isMode1860WalkableTile(fromTileId)) {
        return tiles;
    }

    const Tile& from = board.tileAt(fromTileId);
    const int dRow[] = {-1, 0, 1, 0};
    const int dCol[] = {0, 1, 0, -1};
    for (int i = 0; i < 4; ++i) {
        const int nextTile = board.mode1860TileIdAt(from.mode1860Y + dRow[i], from.mode1860X + dCol[i]);
        if (isLegal1860Step(fromTileId, nextTile)) {
            tiles.push_back(nextTile);
        }
    }
    return tiles;
}

//Input: currentPlayer index and remaining movement points
//Output: selected next tile id, or current tile id if no legal step exists
//Purpose: chooses one CPU step on the 1860 board while favoring retirement progress
//Relation: used by moveCPUManually1860
int Game::chooseCPU1860NextStep(int currentPlayer, int remainingSteps) const {
    const Player& player = players[static_cast<std::size_t>(currentPlayer)];
    const std::vector<int> options = validAdjacent1860Tiles(player.tile);
    if (options.empty()) {
        return player.tile;
    }

    const int retirementTile = board.mode1860RetirementTileId();
    for (std::size_t i = 0; i < options.size(); ++i) {
        if (options[i] == retirementTile) {
            return retirementTile;
        }
    }

    if (player.cpuDifficulty == CpuDifficulty::Easy && options.size() > 1U) {
        const int pick = std::abs(player.tile + remainingSteps + player.cash) % static_cast<int>(options.size());
        return options[static_cast<std::size_t>(pick)];
    }

    int bestTile = options.front();
    int bestScore = -1000000;
    for (std::size_t i = 0; i < options.size(); ++i) {
        const Tile& tile = board.tileAt(options[i]);
        int score = mode1860ProgressScore(board, options[i]) * 8;
        score -= mode1860DistanceToRetirement(board, options[i]) * 5;

        if (player.cpuDifficulty == CpuDifficulty::Hard) {
            switch (tile.kind) {
                case TILE_RETIREMENT:
                    score += 10000;
                    break;
                case TILE_PAYDAY:
                case TILE_SAFE:
                    score += 45;
                    break;
                case TILE_CAREER:
                case TILE_CAREER_2:
                case TILE_COLLEGE:
                case TILE_GRADUATION:
                case TILE_NIGHT_SCHOOL:
                    score += player.job == "Unemployed" ? 55 : 25;
                    break;
                case TILE_RISKY:
                case TILE_SPLIT_RISK:
                    score += player.cash > 120000 ? 10 : -35;
                    break;
                default:
                    break;
            }
        } else if (player.cpuDifficulty == CpuDifficulty::Normal) {
            if (tile.kind == TILE_RISKY || tile.kind == TILE_SPLIT_RISK) {
                score -= 15;
            }
            if (tile.kind == TILE_PAYDAY || tile.kind == TILE_SAFE) {
                score += 15;
            }
        }

        if (score > bestScore || (score == bestScore && options[i] < bestTile)) {
            bestScore = score;
            bestTile = options[i];
        }
    }
    return bestTile;
}

//Input: currentPlayer index and movement steps
//Output: true if the player moved at least one tile
//Purpose: lets a human manually spend 1860 movement points one adjacent tile at a time
//Relation: used by takeMovementSpin and 1860 action-card movement
bool Game::moveHumanManually1860(int currentPlayer, int steps) {
    Player& player = players[static_cast<std::size_t>(currentPlayer)];
    if (!board.isMode1860WalkableTile(player.tile)) {
        player.tile = board.mode1860StartTileId();
    }

    int movementUsed = 0;
    bool moved = false;
    bool effectAppliedOnCurrentTile = false;
    bool resumedAfterStop = false;

    keypad(boardWin, TRUE);

    while (movementUsed < steps && !player.retired) {
        std::vector<int> plannedPath;
        plannedPath.push_back(player.tile);

        int cursorTile = player.tile;
        std::string statusTitle = resumedAfterStop
            ? "1860 movement:"
            : "1860 movement: " + std::to_string(steps - movementUsed) +
                  " point" + ((steps - movementUsed) == 1 ? "" : "s") + " left";
        std::string statusDetail =
            "Use W/A/S/D to plan. ENTER travels the trail. BACKSPACE pulls back. No backward travel after commit.";

        bool cancelled = false;

        while (!player.retired) {
            const int plannedSteps = static_cast<int>(plannedPath.size()) - 1;
            const int remaining = std::max(0, steps - movementUsed - plannedSteps);

            std::vector<int> adjacent;
            if (remaining > 0) {
                adjacent = validAdjacent1860Tiles(cursorTile);
            }

            if (adjacent.empty() && plannedPath.size() == 1U && remaining > 0) {
                showInfoPopup("1860 Movement", "The trail has no legal forward space from here.");
                cancelled = true;
                break;
            }

            std::vector<int> selectable = adjacent;
            if (plannedPath.size() > 1U) {
                selectable.push_back(plannedPath[plannedPath.size() - 2]);
            }

            std::vector<Player> previewPlayers = players;
            previewPlayers[static_cast<std::size_t>(currentPlayer)].tile = cursorTile;

            board.render1860Selection(boardWin,
                                      previewPlayers,
                                      currentPlayer,
                                      cursorTile,
                                      selectable,
                                      remaining,
                                      hasColor);

            const Tile& current = board.tileAt(cursorTile);
            draw_sidebar_ui(infoWin, board, previewPlayers, currentPlayer, history.recent(), rules);
            draw_message_ui(
                msgWin,
                statusTitle,
                "Trail end: Space " + std::to_string(cursorTile) + " - " + getTileDisplayName(current) +
                    " | Planned " + std::to_string(plannedSteps) + "/" +
                    std::to_string(steps - movementUsed) + ". " + statusDetail);

            const int ch = wgetch(boardWin);

            if (isConfirmKey(ch)) {
                break;
            }

            if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
                if (plannedPath.size() > 1U) {
                    plannedPath.pop_back();
                    cursorTile = plannedPath.back();
                    statusTitle = resumedAfterStop ? "1860 movement:" : statusTitle;
                    statusDetail = "You pull the marker back along your uncommitted trail.";
                } else {
                    beep();
                    statusDetail = "There is no planned trail to pull back yet.";
                }
                continue;
            }

            if (ch == 27 || ch == 'q' || ch == 'Q') {
                showInfoPopup("1860 Movement", "Route planning cancelled before the wagon moved.");
                cancelled = true;
                break;
            }

            int dRow = 0;
            int dCol = 0;
            if (ch == KEY_UP || ch == 'w' || ch == 'W') {
                dRow = -1;
            } else if (ch == KEY_RIGHT || ch == 'd' || ch == 'D') {
                dCol = 1;
            } else if (ch == KEY_DOWN || ch == 's' || ch == 'S') {
                dRow = 1;
            } else if (ch == KEY_LEFT || ch == 'a' || ch == 'A') {
                dCol = -1;
            } else {
                continue;
            }

            const Tile& cursor = board.tileAt(cursorTile);
            const int nextTile = board.mode1860TileIdAt(cursor.mode1860Y + dRow, cursor.mode1860X + dCol);

            if (plannedPath.size() > 1U && nextTile == plannedPath[plannedPath.size() - 2]) {
                plannedPath.pop_back();
                cursorTile = plannedPath.back();
                statusTitle = resumedAfterStop ? "1860 movement:" : statusTitle;
                statusDetail = "You backtracked along the trail you have not committed yet.";
                continue;
            }

            if (remaining <= 0) {
                beep();
                statusDetail = "No movement points left. Press ENTER to travel this planned route, or backtrack first.";
                continue;
            }

            if (!isLegal1860Step(cursorTile, nextTile)) {
                beep();
                statusTitle = "1860 rule: no backward travel";
                if (dRow > 0 || dCol < 0) {
                    statusDetail =
                        "You can only move toward Retirement (Top-Right). You may only undo your uncommitted trail.";
                } else {
                    statusDetail = "That square is not part of the open forward trail.";
                }
                continue;
            }

            plannedPath.push_back(nextTile);
            cursorTile = nextTile;
            statusTitle = resumedAfterStop ? "1860 movement:" : statusTitle;
            statusDetail = "Trail marked. Press ENTER to travel it, or step back along your trail before committing.";
        }

        if (cancelled) {
            break;
        }

        if (plannedPath.size() <= 1U) {
            break;
        }

        bool stoppedOnStopTile = false;
        resumedAfterStop = false;

        for (std::size_t step = 1; step < plannedPath.size() && !player.retired; ++step) {
            player.tile = plannedPath[step];
            moved = true;
            ++movementUsed;
            effectAppliedOnCurrentTile = false;

            renderGame(currentPlayer,
                       player.name + " follows the marked trail to " + getTileDisplayName(board.tileAt(player.tile)),
                       "1860 movement committed: step " + std::to_string(step) +
                           " of " + std::to_string(plannedPath.size() - 1) +
                           ". The road still only runs forward toward Retirement.");

            napms(120);
            checkTrapTrigger(currentPlayer);

            if (!board.isMode1860WalkableTile(player.tile)) {
                player.tile = board.mode1860StartTileId();
                break;
            }

            if (player.tile != plannedPath[step]) {
                break;
            }

            if (board.isStopSpace(board.tileAt(player.tile))) {
                showInfoPopup("STOP Tile", "You have landed on a STOP tile!");
                applyTileEffect(currentPlayer, board.tileAt(player.tile));
                effectAppliedOnCurrentTile = true;
                stoppedOnStopTile = true;
                break;
            }
        }

        if (stoppedOnStopTile && movementUsed < steps && !player.retired) {
            resumedAfterStop = true;
            continue;
        }

        break;
    }

    if (moved && !player.retired && !effectAppliedOnCurrentTile) {
        applyTileEffect(currentPlayer, board.tileAt(player.tile));
    }

    return moved;
}

//Input: currentPlayer index and movement steps
//Output: true if the player moved at least one tile
//Purpose: moves a CPU player one legal 1860 step at a time toward Retirement
//Relation: used by takeMovementSpin and 1860 action-card movement
bool Game::moveCPUManually1860(int currentPlayer, int steps) {
    Player& player = players[static_cast<std::size_t>(currentPlayer)];
    if (!board.isMode1860WalkableTile(player.tile)) {
        player.tile = board.mode1860StartTileId();
    }

    bool moved = false;
    bool effectAppliedOnCurrentTile = false;
    for (int remaining = std::max(0, steps); remaining > 0 && !player.retired; --remaining) {
        const int nextTile = chooseCPU1860NextStep(currentPlayer, remaining);
        if (nextTile == player.tile || !isLegal1860Step(player.tile, nextTile)) {
            break;
        }

        player.tile = nextTile;
        moved = true;
        effectAppliedOnCurrentTile = false;
        renderGame(currentPlayer,
                   player.name + " advances on the 1860 board",
                   "CPU movement point " + std::to_string(steps - remaining + 1) +
                       " of " + std::to_string(steps) + ". Goal: Retirement top-right.");
        napms(autoAdvanceUi ? 80 : 180);
        checkTrapTrigger(currentPlayer);
        if (!board.isMode1860WalkableTile(player.tile)) {
            player.tile = board.mode1860StartTileId();
            break;
        }
        if (board.isStopSpace(board.tileAt(player.tile))) {
            applyTileEffect(currentPlayer, board.tileAt(player.tile));
            effectAppliedOnCurrentTile = true;
        }
    }

    if (moved && !player.retired && !effectAppliedOnCurrentTile) {
        applyTileEffect(currentPlayer, board.tileAt(player.tile));
    }
    return moved;
}

std::string Game::movePlayerByAction(int playerIndex, int steps) {
    Player& player = players[playerIndex];
    if (steps == 0) {
        return "No movement.";
    }

    if (boardViewMode == BoardViewMode::Mode1860) {
        if (!board.isMode1860WalkableTile(player.tile)) {
            player.tile = board.mode1860StartTileId();
        }

        const int totalSteps = steps > 0 ? steps : -steps;
        if (steps > 0) {
            const int startTile = player.tile;
            const bool moved = isCpuPlayer(playerIndex)
                ? moveCPUManually1860(playerIndex, totalSteps)
                : moveHumanManually1860(playerIndex, totalSteps);
            if (!moved) {
                return "No legal 1860 movement step was taken.";
            }
            std::ostringstream out;
            out << "Moved on the 1860 board from Space " << startTile
                << " to Space " << player.tile << " - " << getTileDisplayName(board.tileAt(player.tile)) << ".";
            return out.str();
        }

        return "1860 mode does not use classic backward pathing for action-card movement.";
    }

    const int totalSteps = steps > 0 ? steps : -steps;
    int moved = 0;
    bool stoppedOnSpace = false;

    for (int step = 0; step < totalSteps; ++step) {
        int nextTile = -1;
        if (steps > 0) {
            const Tile& current = board.tileAt(player.tile);
            nextTile = chooseNextTile(player, current);
        } else {
            nextTile = findPreviousTile(player, player.tile);
        }

        if (nextTile < 0 || nextTile == player.tile) {
            break;
        }

        player.tile = nextTile;
        ++moved;

        const Tile& landed = board.tileAt(player.tile);
        std::string message = player.name + " moved to " + landed.label;
        if (board.isStopSpace(landed)) {
            message = "STOP! " + player.name + " hit " + landed.label;
        }
        renderGame(playerIndex, message, "Action card movement...");
        napms(200);

        if (board.isStopSpace(landed)) {
            stoppedOnSpace = true;
            applyTileEffect(playerIndex, landed);
            break;
        }
    }

    std::ostringstream out;
    out << "Moved " << (steps > 0 ? "forward " : "back ") << moved
        << (moved == 1 ? " space" : " spaces")
        << " to " << board.tileAt(player.tile).label << ".";
    if (stoppedOnSpace) {
        out << " STOP resolved.";
    }
    return out.str();
}

//Input: Player reference, Tile reference
//Output: None
//Purpose: Resolves a baby event by spinning for 0–3 babies, updating player’s family count.
//Relation: Called when landing on a Baby tile (non-family path). Uses rollSpinner, babiesFromSpin, and updates history/UI.
void Game::resolveBabyStop(Player& player, const Tile& tile) {
    const int playerIndex = findPlayerIndex(player);
    if (!isCpuPlayer(playerIndex)) {
        maybeShowFirstTimeTutorial(TutorialTopic::Baby);
    }
    int spin = rollSpinner("Baby Spin", "Hold SPACE to spin for 0 / 1 / 2 / 3 babies");
    int babies = babiesFromSpin(spin);
    player.kids += babies;
    addHistory(player.name + ": " + babiesLabel(babies) + " on " + tile.label);
    showInfoPopup(tile.label + " resolved", babiesLabel(babies));
}

//Input: Player reference.
//Output: None
//Purpose: Resolves Safe route choice with a guaranteed payout based on spin.
//Relation: Called when landing on Safe route tile. Uses safeRoutePayout, credits bank, updates history/UI.
void Game::resolveSafeRoute(Player& player) {
    int spin = rollSpinner("Safe Route", "Spin for a smaller guaranteed reward");
    int payout = safeRoutePayout(spin);
    bank.credit(player, payout);
    addHistory(player.name + " took Safe route for $" + std::to_string(payout));
    showInfoPopup("Safe Route", "Collected $" + std::to_string(payout) + ".");
}

//Input: Player reference.
//Output: None
//Purpose: Resolves Risky route choice with potential large win or loss.
//Relation: Called when landing on Risky route tile. Uses riskyRoutePayout, applies bank charge/credit, handles loans, updates history/UI.
void Game::resolveRiskyRoute(Player& player) {
    int spin = rollSpinner("Risky Route", "Spin for a big win or painful loss");
    int payout = riskyRoutePayout(spin);
    if (payout >= 0) {
        bank.credit(player, payout);
        addHistory(player.name + " won $" + std::to_string(payout) + " on Risky route");
        showInfoPopup("Risky Route", "You won $" + std::to_string(payout) + ".");
        return;
    }

    PaymentResult payment = bank.charge(player, -payout);
    maybeShowLoanTutorial(findPlayerIndex(player), payment);
    addHistory(appendLoanText(player.name + " lost $" + std::to_string(-payout) + " on Risky route", payment));
    showInfoPopup("Risky Route", appendLoanText("You lost $" + std::to_string(-payout) + ".", payment));
}

//Input: Player index
//Output: None
//Purpose: Handles retirement choice (Millionaire Mansion or Countryside Acres), assigns retirement bonus, updates player state.
//Relation: Called when landing on Retirement tile. Uses CPU/human choice logic, updates history/UI.
void Game::resolveRetirement(int playerIndex) {
    Player& player = players[playerIndex];
    if (player.retired) {
        return;
    }

    int choice = 0;
    if (isCpuPlayer(playerIndex)) {
        choice = cpu.chooseRetirement(player);
        showCpuThinking(playerIndex,
                        choice == 0 ? "CPU chose Millionaire Mansion." : "CPU chose Countryside Acres.");
    } else {
        choice = showRequiredBranchPopup(
            "Choose retirement destination",
            std::vector<std::string>{
                "- Millionaire Mansion",
                "- Countryside Acres"
            },
            'A',
            'B');
    }

    player.retired = true;
    player.retirementHome = choice == 0 ? "Millionaire Mansion (MM)" : "Countryside Acres (CA)";
    player.tile = boardViewMode == BoardViewMode::Mode1860
        ? board.mode1860RetirementTileId()
        : (choice == 0 ? 87 : 88);
    ++retiredCount;
    player.retirementPlace = retiredCount;
    player.retirementBonus = rules.toggles.retirementBonusesEnabled ? retirementBonusForPlace(retiredCount) : 0;

    std::ostringstream line;
    line << "Place " << player.retirementPlace;
    if (player.retirementBonus > 0) {
        line << " bonus $" << player.retirementBonus;
    }
    addHistory(player.name + " retired to " + player.retirementHome);
    showInfoPopup("Retirement: " + player.retirementHome, line.str());
}

//Input: Player reference.
//Output: None
//Purpose: Allows player to buy a house card, charges cost, sets house attributes.
//Relation: Called when landing on House tile. Uses decks.drawHouseCard, bank.charge, updates history/UI, may award pet card.
void Game::buyHouse(Player& player) {
    if (player.hasHouse) {
        showInfoPopup("House", "You already own " + player.houseName + ".");
        return;
    }

    HouseCard house;
    if (!decks.drawHouseCard(house)) {
        showInfoPopup("House Deck", "No house cards are available.");
        return;
    }
    PaymentResult payment = bank.charge(player, house.cost);
    const int playerIndex = findPlayerIndex(player);
    if (!isCpuPlayer(playerIndex)) {
        maybeShowFirstTimeTutorial(TutorialTopic::House);
    }
    maybeShowLoanTutorial(playerIndex, payment);
    player.hasHouse = true;
    player.houseName = house.title;
    player.houseValue = house.saleValue;
    player.finalHouseSaleValue = 0;
    addHistory(appendLoanText(player.name + " bought " + house.title, payment));
    maybeAwardPetCard(player, "House purchase bonus: a pet moved in.");
    showInfoPopup("House: " + house.title,
                  appendLoanText("Paid $" + std::to_string(house.cost) +
                                 ", spin sale base $" + std::to_string(house.saleValue) + ".", payment));
}

//Input: Player reference.
//Output: None
//Purpose: Assigns an investment card to player.
//Relation: Called during setup or investment events. Uses decks.drawInvestCard, updates history/UI.
void Game::assignInvestment(Player& player) {
    InvestCard card;
    if (!decks.drawInvestCard(card) || card.number <= 0) {
        return;
    }

    player.investedNumber = card.number;
    player.investPayout = card.payout;
    const int playerIndex = findPlayerIndex(player);
    if (!isCpuPlayer(playerIndex)) {
        maybeShowFirstTimeTutorial(TutorialTopic::Investment);
    }
    addHistory(player.name + " invested on spinner " + std::to_string(card.number));
}

//Input: Spinner value.
//Output: None
//Purpose: Pays out investments if spinner matches player’s invested number.
//Relation: Called after spins. Uses bank.credit, updates history/UI.
void Game::resolveInvestmentPayouts(int spinnerValue) {
    if (!rules.toggles.investmentEnabled) {
        return;
    }

    std::ostringstream summary;
    bool anyMatch = false;
    for (size_t i = 0; i < players.size(); ++i) {
        if (players[i].investedNumber != spinnerValue || players[i].investPayout <= 0) {
            continue;
        }
        bank.credit(players[i], players[i].investPayout);
        addHistory(players[i].name + " investment matched " + std::to_string(spinnerValue));
        if (anyMatch) {
            summary << " | ";
        }
        summary << players[i].name << " +$" << players[i].investPayout;
        anyMatch = true;
    }

    if (anyMatch) {
        showInfoPopup("Investment payout on spin " + std::to_string(spinnerValue), summary.str());
    }
}

//Input: Player reference, spinner value.
//Output: None
//Purpose: Awards Spin to Win token and prize if spinner lands on 10.
//Relation: Called after spins. Updates history/UI.
void Game::maybeAwardSpinToWin(Player& player, int spinnerValue) {
    if (!rules.toggles.spinToWinEnabled || spinnerValue != 10) {
        return;
    }

    ++player.spinToWinTokens;
    bank.credit(player, rules.spinToWinPrize);
    addHistory(player.name + " triggered Spin to Win");
    showInfoPopup("Spin to Win!", player.name + " gains a token and $" +
                  std::to_string(rules.spinToWinPrize) + ".");
}

//Input: Player reference, reason string.
//Output: None.
//Purpose: Awards a pet card if pets are enabled.
//Relation: Called after house purchase or other events. Uses decks.drawPetCard, updates history/UI
void Game::maybeAwardPetCard(Player& player, const std::string& reason) {
    if (!rules.toggles.petsEnabled) {
        return;
    }

    PetCard pet;
    if (!decks.drawPetCard(pet) || pet.title.empty()) {
        return;
    }

    player.petCards.push_back(pet.title);
    const int playerIndex = findPlayerIndex(player);
    if (!isCpuPlayer(playerIndex)) {
        maybeShowFirstTimeTutorial(TutorialTopic::Pet);
    }
    addHistory(player.name + " adopted a " + pet.title);
    showInfoPopup(player.name + " adopted a " + pet.title, reason);
}

//Input: Player index, Tile reference.
//Output: None.
//Purpose: Applies effects based on tile type (start, college, career, marriage, family, risky/safe, payday, house, retirement, baby, etc.).
//Relation: Central dispatcher for tile effects. Calls specialized functions like resolveBabyStop, resolveSafeRoute, resolveRiskyRoute, buyHouse, resolveRetirement
void Game::applyTileEffect(int playerIndex, const Tile& tile) {
    Player& player = players[playerIndex];
    std::string line = "Keep moving.";

    switch (tile.kind) {
        case TILE_START:
            line = "START: The journey begins.";
            addHistory(player.name + " started the journey");
            break;
        case TILE_BLACK:
            if (boardViewMode == BoardViewMode::Mode1860 && tile.value < 3) {
                playActionCard(playerIndex, tile);
                return;
            }
            playBlackTileMinigame(playerIndex);
            return;
        case TILE_COLLEGE: {
            player.collegeGraduate = false;
            if (player.startChoice == 0) {
                line = "College begins. Tuition was already paid when this route was chosen.";
                addHistory(player.name + " entered college");
            } else {
                PaymentResult payment = bank.charge(player, 100000);
                maybeShowLoanTutorial(playerIndex, payment);
                line = appendLoanText("College opens a late door, but tuition costs $100000.", payment);
                addHistory(appendLoanText(player.name + " entered college", payment));
            }
            break;
        }
        case TILE_CAREER:
            chooseCareer(player, false);
            return;
        case TILE_GRADUATION:
            if (player.startChoice == 0 || player.job == "Unemployed") {
                chooseCareer(player, true);
                return;
            }
            line = "Graduation day passes by. Career-route players keep building the job they already chose.";
            addHistory(player.name + " cleared Graduation");
            break;
        case TILE_MARRIAGE:
            resolveMarriageStop(player);
            return;
        case TILE_FAMILY:
            if (player.hasFamilyPath && player.familyBabyEventsRemaining > 0) {
                triggerBabyEvent(player);
            } 
            else if (player.hasFamilyPath && player.familyBabyEventsRemaining == 0) {
                addHistory(player.name + " passed through Family space - no more baby events remain");  // ADD THIS
                showInfoPopup("Family Space", "Your family journey is complete.");

            } else {
                showInfoPopup("Family Space", "You continue on your journey.");
            }
            return;
        case TILE_NIGHT_SCHOOL:
            resolveNightSchool(player);
            return;
        case TILE_SPLIT_RISK:
            if (!rules.toggles.riskyRouteEnabled) {
                player.riskChoice = 0;
                addHistory(player.name + " defaults to the Safe route");
                showInfoPopup("Risk split", "Risky route is disabled. Safe route selected.");
            } else {
                int riskChoice = 0;
                if (isCpuPlayer(playerIndex)) {
                    riskChoice = cpu.chooseRiskRoute(player, rules);
                    showCpuThinking(playerIndex,
                                    riskChoice == 0 ? "CPU chose Safe route." : "CPU chose Risky route.");
                } else {
                    riskChoice = showRequiredBranchPopup(
                        "Safe or Risky route?",
                        std::vector<std::string>{
                            "- Safe route: smaller payout, no huge swings",
                            "- Risky route: bigger wins and losses"
                        },
                        'A',
                        'B');
                }
                player.riskChoice = riskChoice;
                addHistory(player.name + (riskChoice == 0 ? " chose Safe route" : " chose Risky route"));
                showInfoPopup("Risk split", riskChoice == 0 ? "Safe route selected." : "Risky route selected.");
            }
            return;
        case TILE_SAFE:
            resolveSafeRoute(player);
            return;
        case TILE_RISKY:
            resolveRiskyRoute(player);
            return;
        case TILE_SPIN_AGAIN:
            addHistory(player.name + " hit Spin Again");
            showInfoPopup("Spin Again", "Take another full movement spin right now.");
            takeMovementSpin(playerIndex, "Spin Again");
            return;
        case TILE_CAREER_2:
            player.salary += tile.value;
            line = "A promotion lands at the right time. Salary rises to $" + std::to_string(player.salary) + ".";
            addHistory(player.name + " got a promotion");
            break;
        case TILE_PAYDAY: {
            int payout = tile.value + effectiveSalary(player);
            bank.credit(player, payout);
            line = "Payday arrives. " + player.name + " collects $" + std::to_string(payout) + ".";
            if (player.salaryReductionTurns > 0) {
                line += " Salary sabotage reduced this payday.";
            }
            addHistory(player.name + " collected payday $" + std::to_string(payout));
            break;
        }
        case TILE_HOUSE:
            buyHouse(player);
            return;
        case TILE_RETIREMENT:
            resolveRetirement(playerIndex);
            return;
        case TILE_BABY:
            if (player.hasFamilyPath && player.familyBabyEventsRemaining > 0) {
                triggerBabyEvent(player);
            } else {
                resolveBabyStop(player, tile); // Original behavior for non-family path players
            }
            return;
        case TILE_SPLIT_START:
        case TILE_SPLIT_FAMILY: //first time player hits family split
            resolveFamilyStop(player); //ask: family path vs life path
            return;
        case TILE_EMPTY:
        default:
            break;
    }

    showInfoPopup(getTileDisplayName(tile) + " resolved", line);
}

//Input: Player reference, Tile reference.
//Output: Next tile ID.
//Purpose: Determines next tile based on player’s choices (college/career, family/life, safe/risky).
//Relation: Used in movement logic (animateMove)
int Game::chooseNextTile(Player& player, const Tile& tile) {
    if ((tile.kind == TILE_SPLIT_START || tile.kind == TILE_START) && player.startChoice == -1) {
        const int playerIndex = findPlayerIndex(player);
        int c = 0;
        if (isCpuPlayer(playerIndex)) {
            c = cpu.chooseStartRoute(player, rules);
            showCpuThinking(playerIndex,
                            c == 0
                                ? "CPU weighs early debt against stronger future jobs, then chooses College."
                                : "CPU chooses Career to start earning sooner.");
        } else {
            c = showRequiredBranchPopup(
                "College or Career?",
                std::vector<std::string>{
                    "- College: pay $100000 now, stronger jobs later",
                    "- Career: choose a job right away"
                },
                'A',
                'B');
        }
        player.startChoice = c;
        if (c == 0) {
            PaymentResult payment = bank.charge(player, 100000);
            maybeShowLoanTutorial(playerIndex, payment);
            player.collegeGraduate = false;
            addHistory(appendLoanText(player.name + " chose College and paid $100000", payment));
            showDecisionPopup(player.name,
                              "College Route",
                              appendLoanText("A bigger future starts with a bigger bill. Tuition is paid before moving onto College Route (CO).", payment),
                              hasColor,
                              autoAdvanceUi);
        } else {
            addHistory(player.name + " chose Career");
            showDecisionPopup(player.name,
                              "Career Path",
                              "The faster road begins now: choose a job immediately and start earning salary sooner.",
                              hasColor,
                              autoAdvanceUi);
        }
    }

    if (tile.kind == TILE_SPLIT_FAMILY) {
        if (player.familyChoice == -1) {
            player.familyChoice = 1;
        }
        return player.familyChoice == 0 ? tile.next : tile.altNext;
    }

    if (tile.kind == TILE_SPLIT_RISK) {
        if (player.riskChoice == -1) {
            player.riskChoice = 0;
        }
        return player.riskChoice == 0 ? tile.next : tile.altNext;
    }

    if (tile.kind == TILE_SPLIT_START || tile.kind == TILE_START) {
        return player.startChoice == 0 ? tile.next : tile.altNext;
    }

    return tile.next;
}

//Input: Current player index, number of steps.
//Output: Boolean (true if stopped early at a stop space).
//Purpose: Animates player movement across tiles, applies traps and tile effects.
//Relation: Called during movement spins. Uses chooseNextTile, applyTileEffect, checkTrapTrigger
bool Game::animateMove(int currentPlayer, int steps) {
    Player& player = players[currentPlayer];
    for (int step = 0; step < steps; ++step) {
        const Tile& current = board.tileAt(player.tile);
        int nextTile = chooseNextTile(player, current);
        if (nextTile < 0) break;
        player.tile = nextTile;

        const Tile& landed = board.tileAt(player.tile);
        std::string message = player.name + " moved to " + getTileDisplayName(landed);
        if (board.isStopSpace(landed)) {
            message = "STOP! " + player.name + " hit " + getTileDisplayName(landed);
        }
        renderGame(currentPlayer, message, "Movement in progress...");
        napms(200);
        checkTrapTrigger(currentPlayer);
        const Tile& afterTrap = board.tileAt(player.tile);

        if (board.isStopSpace(afterTrap)) {
            applyTileEffect(currentPlayer, afterTrap);
            return true;
        }
    }
    return false;
}

//Input: Current player index, reason string.
//Output: None.
//Purpose: Executes a movement spin, applies penalties, awards bonuses, animates movement, applies tile effects, shows summary.
//Relation: Core turn mechanic. Calls rollSpinner, animateMove, applyTileEffect, showTurnSummaryPopup
void Game::takeMovementSpin(int currentPlayer, const std::string& reason) {
    Player& player = players[currentPlayer];
    if (boardViewMode == BoardViewMode::Mode1860 && !board.isMode1860WalkableTile(player.tile)) {
        player.tile = board.mode1860StartTileId();
    }
    const int startTile = player.tile;
    const int startingCash = player.cash;
    const int startingLoans = player.loans;
    const Tile& start = board.tileAt(startTile);
    std::string typeLine = playerTypeLabel(player.type);
    if (player.type == PlayerType::CPU) {
        typeLine += " " + cpuDifficultyLabel(player.cpuDifficulty);
    }
    showInfoPopup(player.name + "'s Turn - " + typeLine,
                  "Money $" + std::to_string(player.cash) + " | Space " +
                  std::to_string(startTile) + " - " + getTileDisplayName(start));

    int roll = rollSpinner(reason, "Hold SPACE to spin movement");
    const int originalRoll = roll;
    addHistory(player.name + " spun " + std::to_string(roll));
    maybeAwardSpinToWin(player, roll);
    resolveInvestmentPayouts(roll);

    if (player.movementPenaltyTurns > 0 && player.movementPenaltyPercent > 0) {
        if (player.movementPenaltyPercent >= 100) {
            roll = 0;
        } else {
            roll = std::max(1, (roll * (100 - player.movementPenaltyPercent)) / 100);
        }
        addHistory(player.name + " movement reduced from " +
                   std::to_string(originalRoll) + " to " + std::to_string(roll));
        showInfoPopup("Traffic Jam", "Movement reduced from " +
                      std::to_string(originalRoll) + " to " + std::to_string(roll) + ".");
    }

    if (roll <= 0) {
        addHistory(player.name + " could not move because of sabotage");
        showInfoPopup("Traffic Jam", "Movement cancelled. No tile effect is resolved.");
        showTurnSummaryPopup(currentPlayer,
                             originalRoll,
                             roll,
                             startTile,
                             player.tile,
                             startingCash,
                             startingLoans,
                             reason);
        return;
    }

    if (boardViewMode == BoardViewMode::Mode1860 && !isCpuPlayer(currentPlayer)) {
        showInfoPopup("1860 Movement",
                      "You have " + std::to_string(roll) +
                          " movement. Use W/A/S/D to move around. You can only move toward Retirement (Top-Right) and cannot move backwards.");
    }

    if (boardViewMode == BoardViewMode::Mode1860) {
        if (isCpuPlayer(currentPlayer)) {
            moveCPUManually1860(currentPlayer, roll);
        } else {
            moveHumanManually1860(currentPlayer, roll);
        }
    } else {
        const bool stoppedEarly = animateMove(currentPlayer, roll);
        if (!stoppedEarly) {
            applyTileEffect(currentPlayer, board.tileAt(player.tile));
        }
    }
    showTurnSummaryPopup(currentPlayer,
                         originalRoll,
                         roll,
                         startTile,
                         player.tile,
                         startingCash,
                         startingLoans,
                         reason);
}
    
//Input: None.
//Output: Boolean (true if all players retired).
//Purpose: Checks if game should end.
//Relation: Used in main game loop (run)
bool Game::allPlayersRetired() const {
    for (size_t i = 0; i < players.size(); ++i) {
        if (!players[i].retired) {
            return false;
        }
    }
    return true;
}

//Input: None.
//Output: Boolean (false if quit).
//Purpose: Main game loop: handles start screen, loading/saving, player turns, sabotage, movement, retirement, endgame scoring.
//Relation: Central orchestrator of gameplay. Calls nearly all other functions, manages windows, history, scoring, and UI

// Enum to track game state for better control flow in the main loop
enum class GameState {
    RUNNING,
    EXIT_TO_MENU,
    EXIT_GAME
};


bool Game::run() {
    if (!ensureMinSize()) return false;

    while (true) {
        while (true) {
            const StartChoice startChoice = showStartScreen();
            if (startChoice == START_QUIT_GAME) {
                return false;
            }

            if (startChoice == START_LOAD_GAME) {
                createWindows(); //Very heavy duty function
                if (loadSavedGame()) {
                    break;
                }
                destroyWindows(); //Very heavy duty function
                continue;
            }

            createWindows();

            setupRules();
            if (!setupPlayers()) {
                destroyWindows();
                continue;
            }

            setupInvestments();
            if (rules.toggles.tutorialEnabled) {
                showPreGameQuickGuide(hasColor, boardViewMode == BoardViewMode::Mode1860);
                addHistory("Quick guide reviewed");
            }
            break;
        }
        GameState gameState = GameState::RUNNING; // Track whether we are still running the game or need to exit to menu or quit

        while (gameState == GameState::RUNNING && !allPlayersRetired()) {
            if (!ensureMinSize()) return false;
            
            if (!windowsValid){
                destroyWindows(); //TODO number 3 - avoid lag by only triggreing destroy/create when actually needed, not every turn after a resize
                createWindows();  
            }

            if (players[currentPlayerIndex].retired) {
                currentPlayerIndex = (currentPlayerIndex + 1) % static_cast<int>(players.size());
                continue;
            }

            if (resolveSkipTurn(currentPlayerIndex)) {
                ++players[currentPlayerIndex].turnsTaken;
                ++turnCounter;
                currentPlayerIndex = (currentPlayerIndex + 1) % static_cast<int>(players.size());
                continue;
            }

            renderGame(currentPlayerIndex,
                       players[currentPlayerIndex].name + "'s turn",
                       turnPromptText());
            maybeShowSabotageUnlock(currentPlayerIndex);
            maybeCpuSabotage(currentPlayerIndex);
            int command = waitForTurnCommand(currentPlayerIndex);

            if (isCancelKey(command)) {
                const int quitChoice = showBranchPopup(
                    "Paused",
                    std::vector<std::string>{
                        "- Back to game",
                        "- Save and quit",
                        "- Quit without saving"
                    },
                    'A',
                    'B');

                if (quitChoice == MENU_CANCELLED || quitChoice == 0) { //Back to game
                    renderGame(currentPlayerIndex,
                               players[currentPlayerIndex].name + "'s turn",
                               turnPromptText());
                    continue;
                }
                if (quitChoice == 1) { //Save and return to menu
                    saveCurrentGame();
                    gameState = GameState::EXIT_TO_MENU;
                    break;
                }
                if (quitChoice == 2) { //Quit without saving
                    gameState = GameState::EXIT_GAME;
                    break;
                }
                continue;
            }
            if (command == 's' || command == 'S') {
                saveCurrentGame();
                continue;
            }
            if (command == 'b' || command == 'B') {
                const bool sabotageUsed = promptSabotageMenu(currentPlayerIndex);
                renderGame(currentPlayerIndex,
                           players[currentPlayerIndex].name + "'s turn",
                           turnPromptText());
                if (!sabotageUsed) {
                    continue;
                }
            }

            autoAdvanceUi = isCpuPlayer(currentPlayerIndex);
            takeMovementSpin(currentPlayerIndex, "Movement Spin");
            autoAdvanceUi = false;
            decrementTurnStatuses(players[currentPlayerIndex]);
            ++players[currentPlayerIndex].turnsTaken;
            ++turnCounter;
            currentPlayerIndex = (currentPlayerIndex + 1) % static_cast<int>(players.size());
        }

        // Normal game completion (all players retired)
        maybeShowFirstTimeTutorial(TutorialTopic::EndgameScoring);
        finalizeScoring();

        int winner = 0;
        int best = calculateFinalWorth(players[0]);
        for (size_t i = 1; i < players.size(); ++i) {
            int worth = calculateFinalWorth(players[i]);
            if (worth > best) {
                best = worth;
                winner = static_cast<int>(i);
            }
        }

        // TODO number 2: final score breakdown

        //const int winnerHouse = players[winner].finalHouseSaleValue > 0
        //    ? players[winner].finalHouseSaleValue
        //    : players[winner].houseValue;

        // Build ranked order by final worth
        std::vector<std::size_t> ranked;
        for (std::size_t i = 0; i < players.size(); ++i) {
            ranked.push_back(i);
        }
        std::sort(ranked.begin(), ranked.end(), [&](std::size_t a, std::size_t b) {
            return calculateFinalWorth(players[a]) > calculateFinalWorth(players[b]);
        });

        std::vector<std::string> endgameLines;
        endgameLines.push_back(players[winner].name + " reaches the finish with the highest final score: $" +
                               std::to_string(best) + ".");
        endgameLines.push_back("");

        // Per-player breakdown, ranked 1st to last
        for (std::size_t rank = 0; rank < ranked.size(); ++rank) {
            const Player& p = players[ranked[rank]];
            const int house = p.finalHouseSaleValue > 0 ? p.finalHouseSaleValue : p.houseValue;
            const int actionScore  = static_cast<int>(p.actionCards.size()) * 100000;
            const int petScore     = static_cast<int>(p.petCards.size()) * 100000;
            const int familyScore  = p.kids * 50000;
            const int loanPenalty  = bank.totalLoanDebt(p);
            const int finalWorth   = calculateFinalWorth(p);
            const std::string medal = (rank == 0) ? "1st" : (rank == 1) ? "2nd" : (rank == 2) ? "3rd" : "4th";

            endgameLines.push_back(medal + " " + p.name + "  TOTAL: $" + std::to_string(finalWorth));
            endgameLines.push_back("  Cash $" + std::to_string(p.cash) +
                                   "  House $" + std::to_string(house) +
                                   "  Actions $" + std::to_string(actionScore));
            endgameLines.push_back("  Pets $" + std::to_string(petScore) +
                                   "  Family $" + std::to_string(familyScore) +
                                   "  Retire bonus $" + std::to_string(p.retirementBonus));
            endgameLines.push_back("  Loans -$" + std::to_string(loanPenalty));
            endgameLines.push_back("");
        }
        endgameLines.push_back("Press ENTER to return to the main menu.");

        appendCompletedGameHistoryEntry(winner, best);
        addHistory("Completed game: " + players[winner].name + " won with $" + std::to_string(best));
        showPopupMessage("Player " + std::to_string(winner + 1) + " Wins!",
                         endgameLines,
                         hasColor,
                         false);

        destroyWindows();
        players.clear();
        currentPlayerIndex = 0;
        retiredCount = 0;
        turnCounter = 0;
        activeTraps.clear();
        history.clear();
        clear();
        refresh();
    }
}
