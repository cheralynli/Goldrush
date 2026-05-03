#include "game.hpp"
#include "pong.hpp"
#include "battleship.hpp"
#include "hangman.hpp"
#include "memory.hpp"
#include "minesweeper.hpp"
#include "ui.h"
#include "ui_helpers.h"
#include "tutorials.h"
#include "spins.hpp"

#include <sstream>
#include <string>

void Game::playBlackTileMinigame(int playerIndex) {
    Player& player = players[playerIndex];
    // Now 5 minigames: Pong (0), Battleship (1), Hangman (2), Memory (3), Minesweeper (4)
    const int minigameChoice = rng.uniformInt(0, 4);

    if (!isCpuPlayer(playerIndex)) {
        maybeShowFirstTimeTutorial(TutorialTopic::Minigame);
    }

    if (titleWin) touchwin(titleWin);
    if (boardWin) touchwin(boardWin);
    if (infoWin) touchwin(infoWin);
    if (msgWin) touchwin(msgWin);

    if (isCpuPlayer(playerIndex)) {
        const CpuMinigameResult cpuResult = cpu.playBlackTileMinigame(player, minigameChoice);
        const int payout = cpuResult.score * 100;
        if (payout > 0) {
            bank.credit(player, payout);
        }

        static const char* names[] = {
            "Pong",
            "Battleship",
            "Hangman",
            "Memory Match",
            "Minesweeper"
        };
        const std::string minigameName = names[minigameChoice];
        addHistory(player.name + " simulated " + minigameName + " and earned $" +
                   std::to_string(payout));
        renderGame(playerIndex,
                   player.name + " completed CPU " + minigameName,
                   cpuResult.summary + " | Earned $" + std::to_string(payout));
        showInfoPopup("CPU BLACK TILE: " + minigameName,
                      cpuResult.summary + " | Earned $" + std::to_string(payout));
        return;
    }

    //Player chooses PONG
    if (minigameChoice == 0) {
        addHistory(player.name + " landed on a black tile and entered Pong");
        showInfoPopup("BLACK TILE: PONG",
                      "One life. Each paddle return earns $100. Press ENTER to start.");

        const PongMinigameResult result = playPongMinigame(player.name, hasColor);

        if (titleWin) touchwin(titleWin);
        if (boardWin) touchwin(boardWin);
        if (infoWin) touchwin(infoWin);
        if (msgWin) touchwin(msgWin);

        if (result.abandoned) {
            addHistory(player.name + " left Pong before finishing");
            renderGame(playerIndex, player.name + " left the Pong sidegame", "No payout awarded");
            showInfoPopup("PONG sidegame", "Exited early. No payout awarded.");
            return;
        }

        const int payout = result.playerScore * 100;
        if (payout > 0) {
            bank.credit(player, payout);
        }

        std::ostringstream summary;
        summary << "Score " << result.playerScore
                << " | CPU " << result.cpuScore
                << " | Earned $" << payout;

        addHistory(player.name + " scored " + std::to_string(result.playerScore) +
                   " in Pong and earned $" + std::to_string(payout));
        renderGame(playerIndex, player.name + " finished the Pong sidegame", summary.str());
        showInfoPopup("PONG sidegame", summary.str());
        return;
    }

    //Player chooses BATTLESHIP
    else if (minigameChoice == 1) {
        addHistory(player.name + " landed on a black tile and entered Battleship");
        showInfoPopup("BLACK TILE: BATTLESHIP",
                    "Shoot the $ ships. One enemy hit ends the run. Each ship is worth $100.");

        const BattleshipMinigameResult result = playBattleshipMinigame(player.name, hasColor);

        if (titleWin) touchwin(titleWin);
        if (boardWin) touchwin(boardWin);
        if (infoWin) touchwin(infoWin);
        if (msgWin) touchwin(msgWin);

        if (result.abandoned) {
            addHistory(player.name + " left Battleship before finishing");
            renderGame(playerIndex, player.name + " left the Battleship sidegame", "No payout awarded");
            showInfoPopup("BATTLESHIP sidegame", "Exited early. No payout awarded.");
            return;
        }

        const int payout = result.shipsDestroyed * 100;
        if (payout > 0) {
            bank.credit(player, payout);
        }

        std::ostringstream summary;
        summary << "Destroyed " << result.shipsDestroyed
                << " ships"
                << (result.clearedWave ? " | Wave cleared" : " | Wave failed")
                << " | Earned $" << payout;

        addHistory(player.name + " destroyed " + std::to_string(result.shipsDestroyed) +
                " ships in Battleship and earned $" + std::to_string(payout));
        renderGame(playerIndex, player.name + " finished the Battleship sidegame", summary.str());
        showInfoPopup("BATTLESHIP sidegame", summary.str());
        return;
    }

    //PLayer chooses HANGMAN
    else if (minigameChoice == 2) {
        addHistory(player.name + " landed on a black tile and entered Hangman");
        showInfoPopup("BLACK TILE: HANGMAN",
                    "Guess the word. Hint unlocks after 5 misses. Each revealed letter is worth $100.");
        
        const HangmanResult result = playHangmanMinigame(player.name, hasColor);
        
        if (titleWin) touchwin(titleWin);
        if (boardWin) touchwin(boardWin);
        if (infoWin) touchwin(infoWin);
        if (msgWin) touchwin(msgWin);
        
        if (result.abandoned) {
            addHistory(player.name + " left Hangman before finishing");
            renderGame(playerIndex, player.name + " left the Hangman sidegame", "No payout awarded");
            showInfoPopup("HANGMAN sidegame", "Exited early. No payout awarded.");
            return;
        }
        
        const int payout = result.lettersGuessed * 100;
        if (payout > 0) {
            bank.credit(player, payout);
        }

        std::ostringstream summary;
        if (result.won) {
            summary << "Word guessed! Letters revealed: " << result.lettersGuessed
                    << " | Earned $" << payout;
        } else {
            summary << "Hangman completed | Letters revealed: " << result.lettersGuessed
                    << " | Earned $" << payout;
        }

        addHistory(player.name + (result.won
                   ? " won Hangman and earned $" + std::to_string(payout)
                   : " lost Hangman and earned $" + std::to_string(payout)));
        renderGame(playerIndex, player.name + " finished the Hangman sidegame", summary.str());
        showInfoPopup("HANGMAN sidegame", summary.str());
        return;
    }   
    
    //Player chooses MEMORY MATCH
    if (minigameChoice == 3) {
        addHistory(player.name + " landed on a black tile and entered Memory Match");
        showInfoPopup("BLACK TILE: MEMORY MATCH",
                      "Match all 8 pairs. Each wrong match costs a life. 20 lives total. Help button reveals grid.");
        
        const MemoryMatchResult result = playMemoryMatchMinigame(player.name, hasColor);
        
        if (titleWin) touchwin(titleWin);
        if (boardWin) touchwin(boardWin);
        if (infoWin) touchwin(infoWin);
        if (msgWin) touchwin(msgWin);
        
        if (result.abandoned) {
            addHistory(player.name + " left Memory Match before finishing");
            renderGame(playerIndex, player.name + " left the Memory Match sidegame", "No payout awarded");
            showInfoPopup("MEMORY MATCH sidegame", "Exited early. No payout awarded.");
            return;
        }
        
        const int payout = (result.pairsMatched * 100) + (result.won ? 200 : 0);
        if (payout > 0) {
            bank.credit(player, payout);
        }
        
        std::ostringstream summary;
        summary << "Pairs matched: " << result.pairsMatched << "/8"
                << " | Lives left: " << result.livesRemaining
                << " | Earned $" << payout;
        
        addHistory(player.name + " matched " + std::to_string(result.pairsMatched) +
                   " pairs in Memory Match and earned $" + std::to_string(payout));
        renderGame(playerIndex, player.name + " finished the Memory Match sidegame", summary.str());
        showInfoPopup("MEMORY MATCH sidegame", summary.str());
        return;
    }

    if (minigameChoice == 4) {
        const int minesweeperSafeTileTotal = 15;
        addHistory(player.name + " landed on a black tile and entered Minesweeper");
        showInfoPopup("BLACK TILE: MINESWEEPER",
                      "Reveal safe tiles for 60 seconds. One bomb ends the run. Each safe tile is worth $100.");

        const MinesweeperResult result = playMinesweeperMinigame(player.name, hasColor);

        if (titleWin) touchwin(titleWin);
        if (boardWin) touchwin(boardWin);
        if (infoWin) touchwin(infoWin);
        if (msgWin) touchwin(msgWin);

        if (result.abandoned) {
            addHistory(player.name + " left Minesweeper before finishing");
            renderGame(playerIndex, player.name + " left the Minesweeper sidegame", "No payout awarded");
            showInfoPopup("MINESWEEPER sidegame", "Exited early. No payout awarded.");
            return;
        }

        const int payout = result.safeTilesRevealed * 100;
        if (payout > 0) {
            bank.credit(player, payout);
        }

        std::ostringstream summary;
        summary << "Safe tiles: " << result.safeTilesRevealed << "/" << minesweeperSafeTileTotal;
        if (result.hitBomb) {
            summary << " | Bomb hit";
        } else if (result.safeTilesRevealed >= minesweeperSafeTileTotal) {
            summary << " | Board cleared";
        } else {
            summary << " | Time up";
        }
        summary << " | Earned $" << payout;

        addHistory(player.name + " cleared " + std::to_string(result.safeTilesRevealed) +
                   " safe tiles in Minesweeper and earned $" + std::to_string(payout));
        renderGame(playerIndex, player.name + " finished the Minesweeper sidegame", summary.str());
        showInfoPopup("MINESWEEPER sidegame", summary.str());
        return;
    }
}

