# Goldrush - Terminal Edition

A text-based board game inspired by *The Game of Life*, built with C++ and `ncurses`.

## Features

- Branching path system with college/career, family/life, and safe/risky choices
- 2-4 players (Human or CPU with Easy/Normal/Hard difficulty)
- Terminal UI with ASCII title screen, board view (Follow Camera or Classic Full), sidebar, and popup windows
- Normal mode and custom mode rule toggles
- Save/load support through .sav files (relative path saves/)
- Action, career, house, investment, and pet cards
- Deterministic deck and RNG state restoration when loading a saved game
- Turn Summary - Detailed breakdown of each turn's events and stat changes
- Completed Game History - Track past game results, winners, and final scores
- First-Time Tutorials - One-time popups explaining game mechanics as they appear
- Minigame Tutorials - One-page guides before each minigame explaining controls and rewards
- Memory Match Countdown - Large visible countdown during memorization phase with ESC to quit
- Sabotage System - Unlocks on Turn 3, includes traps, lawsuits, forced duels, and defense items
- Board View Modes - Toggle between Follow Camera (zoom) and Classic Full Board

## Game Flow

TITLE SCREEN
  |
  +-- New Game
  |     |
  |     +-- Normal Mode / Custom Mode
  |     |
  |     +-- Player Setup (Human/CPU with difficulty)
  |     |
  |     +-- Pre-game Quick Guide (optional)
  |     |
  |     +-- Main Game Loop
  |
  +-- Load Game (from saves/ directory)
  |
  +-- Quit

Main board progression:

START
  |
  +-- COLLEGE -----+
  |                |
  +-- CAREER ------+
         |
     GRADUATION
         |
      MARRIAGE
         |
     FAMILY STOP
      /       \
  FAMILY      LIFE
    PATH      PATH
      \       /
       HOUSE / PAYDAY / ACTION
             |
         RISK SPLIT
         /       \
      SAFE      RISKY
         \       /
          RETIREMENT

## Space Effects

| Space | Effect |
|-------|--------|
| START | Begin the journey |
| COLLEGE | Pay tuition and head toward degree-based careers |
| CAREER | Start working immediately |
| GRADUATION | Choose or confirm a career path |
| MARRIAGE | Resolve marriage and gift spin |
| FAMILY STOP | Choose family path or life path |
| ACTION | Draw and resolve an action card |
| PAYDAY | Collect salary |
| HOUSE | Buy a house |
| SAFE | Take a lower-risk reward spin |
| RISKY | Take a higher-risk reward/loss spin |
| RETIREMENT | End active play and lock in retirement bonuses |

## Turn Summary

After each turn, a popup shows movement roll, starting/ending positions, money/loan changes, tile effect, and important events.

## Completed Game History

Access history from title screen with H. Records date, winner, all players' scores, game mode, and turn count.

## First-Time Tutorials

Topics: Automatic Loans, Manual Loans, Investments, Jobs, Babies, Pets, Marriage, Houses, Insurance, Shields, Action Cards, Minigames, Sabotage, Endgame Scoring.

## Minigames

| Minigame | Controls | Payout |
|----------|----------|--------|
| Pong | W/S or arrows move, X serves | $100 per paddle return |
| Battleship | A/D or arrows move, Space/Enter fire, R reload | $100 per ship destroyed |
| Hangman | Type A-Z to guess, ESC exits | $100 per letter revealed |
| Memory Match | WASD/arrows move, Enter/Space select, H help | $100 per pair + $200 bonus |
| Minesweeper | WASD/arrows move, Enter/Space reveal | $100 per safe tile |

## Memory Match Countdown

5 second memorization phase with countdown timer. ESC shows quit confirmation. Help button (H) reveals grid for 1 second (5 uses max).

## Sabotage System (Unlocks Turn 3)

| Action | Cost | Effect |
|--------|------|--------|
| Trap Tile | $12,000 | Place trap on any tile |
| Lawsuit | $15,000 | Roll against target |
| Traffic Jam | $10,000 | Reduce target's movement |
| Steal Action Card | $18,000 | Steal card from target |
| Forced Duel | $22,000 | Force minigame duel |
| Career Sabotage | $24,000 | Reduce target's salary |
| Position Swap | $90,000 | Swap tiles with target |
| Debt Trap | $20,000 | Force target to take a loan |
| Buy Shield | $15,000 | Block one sabotage |
| Buy Insurance | $20,000 | Reduce next 2 hits |
| Item Disable | $16,000 | Disable target's items |

## Board View Modes

- Follow Camera: Zoomed view centered on current player
- Classic Full: Full route overview with classic symbols

## Mode Presets

- Normal Mode: All optional systems enabled
- Custom Mode: Toggle individual features
- Relax Mode: Easier settings
- Life Mode: Balanced gameplay
- Hell Mode: Higher difficulty

## Winning

Final worth = cash + house value + action cards + pet cards + baby bonuses + retirement bonus - loan penalties. Highest wins with ranked breakdown.

## Requirements

- C++17 or later
- ncurses
- Unix-like terminal (including WSL on Windows)

Install ncurses:
- macOS: brew install ncurses
- Ubuntu/Debian/WSL: sudo apt-get install libncurses5-dev libncursesw5-dev

## Build

make          # Build main game
make debug    # Build debug suite
make run      # Run main game
make run-debug # Run debug suite
make clean    # Clean build files

## Save / Load

Save files stored in saves/ (relative to executable). Preserves rules, player state, turn counter, history, decks, RNG state, and board view mode.

## Controls

| Key | Action |
|-----|--------|
| N | New game |
| L | Load game |
| H | History |
| G | Guide |
| K or ? | Controls |
| ESC | Back/Cancel/Quit |
| Enter | Confirm/Start turn |
| Space | Spin wheel |
| B | Sabotage menu |
| Tab | Scoreboard + minimap |
| S | Save game |
| Up/Down | Navigate menus |
| Left/Right | Switch modes |

## Project Structure

- main.cpp - Entry point
- game.hpp/cpp - Main game flow
- board.hpp/cpp - Board and view modes
- player.hpp/cpp - Player data
- cards.hpp/cpp - Card systems
- cpu_player.hpp/cpp - AI players
- sabotage.h/cpp, sabotage_card.h/cpp - Sabotage system
- save_manager.hpp/cpp - Save/load
- ui.h/cpp, ui_helpers.h/cpp - UI helpers
- rules.hpp/cpp - Rulesets
- random_service.hpp - RNG
- turn_summary.h/cpp - Turn popups
- completed_history.h/cpp - Game history
- tutorials.h/cpp, minigame_tutorials.h/cpp - Tutorials
- timer_display.h/cpp - Countdown timer
- tile_display.h/cpp - Tile display
- input_helpers.h/cpp - Input handling
- pong.hpp/cpp, battleship.hpp/cpp, hangman.hpp/cpp, memory.hpp/cpp, minesweeper.hpp/cpp - Minigames
- Makefile - Build targets

## Contributors

Cheralyn, Michelle, Yin, Carla, Joylin

## Notes

Educational project. The Game of Life is a trademark of Hasbro.

Enjoy! May the best life win! 🎉