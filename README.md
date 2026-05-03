# Goldrush - Terminal Edition

A text-based board game inspired by *The Game of Life*, built with C++ and `ncurses`.

## Features

- **Dual Board Modes** - Classic Life-style board OR 1860 checkered free-movement board
- **Branching Path System** - College/Career, Family/Life, and Safe/Risky choices
- **2-4 Players** - Human or CPU with Easy/Normal/Hard difficulty levels
- **Terminal UI** - ASCII art, color themes, popup windows, and dynamic camera views
- **Normal & Custom Modes** - Toggle individual game features on/off
- **Save/Load System** - Persistent saves with duplicate detection and archiving
- **Card Systems** - Action, Career, House, Investment, and Pet cards
- **Complete RNG Restoration** - Deterministic deck and RNG state when loading saves
- **Turn Summary** - Detailed breakdown of each turn's events and stat changes
- **Completed Game History** - Track past results, winners, and final scores
- **First-Time Tutorials** - One-time popups explaining mechanics as they appear
- **5 Minigames** - Pong, Battleship, Hangman, Memory Match, Minesweeper
- **Sabotage System** - Unlocks on configurable turn (default 3), includes traps, lawsuits, forced duels, and defense items
- **1860 Free Movement** - Grid-based board where you spend movement points each turn

## Game Flow

TITLE SCREEN
  |
  +-- New Game
  |     |
  |     +-- Board Mode Selection (1860 / Follow Camera / Classic Full)
  |     |
  |     +-- Normal Mode / Custom Mode
  |     |
  |     +-- Player Setup (2-4 players, Human/CPU with difficulty)
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

## Board View Modes

| Mode | Description |
|------|-------------|
| **1860 Mode** | Grid-based checkered board (25x25). Start at bottom-left, reach Retirement at top-right. Movement points spent tile-by-tile each turn. |
| **Follow Camera** | Zoomed view centered on current player, shows 3x5 tile window with connections. |
| **Classic Full** | Complete route overview with traditional board layout and symbols. |

### 1860 Mode Movement

- Each spin gives maximum movement points you may spend that turn
- Move one adjacent tile per point (up, down, left, right - no diagonals)
- You may stop before spending all points
- Landing on an event/stop tile triggers its effect, then you may continue moving
- Large spins give more route flexibility instead of forcing hard stops

### 1860 Tile Colors & Symbols

| Color | Meaning | Symbol |
|-------|---------|--------|
| Green | Payday / Safe | + |
| Teal | Action / College | A |
| Mauve | Minigame / Family | M |
| Red | Risk | ! |
| Blue | Career / Job | J |
| Gold | Start / Route / Retirement | S / R |

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

After each turn, a popup shows:
- Movement roll (original and after penalties)
- Starting/ending tile positions
- Money and loan changes
- Tile effect description
- Important events (marriage, job changes, etc.)

## Completed Game History

Access history from title screen with `H`. Each entry includes:
- Date and time completed
- Winner name and final score
- All players' net worth, cash, loans
- Game mode and settings
- Turn count
- Detailed final ranking with breakdowns per player

## First-Time Tutorials

Tutorials appear once when you first encounter each mechanic:
- Automatic Loans
- Manual Loans
- Investments
- Jobs / Careers
- Babies / Family Path
- Pets
- Marriage
- Houses
- Insurance
- Shields
- Action Cards
- Minigames
- Sabotage
- Endgame Scoring

## Minigames

| Minigame | Controls | Payout |
|----------|----------|--------|
| Pong | W/S or arrows move, X serves | $100 per paddle return |
| Battleship | A/D or arrows move, Space/Enter fire, R reload | $100 per ship destroyed |
| Hangman | Type A-Z to guess, ESC exits | $100 per letter revealed |
| Memory Match | WASD/arrows move, Enter/Space select, help | $100 per pair + $200 bonus |
| Minesweeper | WASD/arrows move, Enter/Space reveal | $100 per safe tile |

## Memory Match Countdown

5 second memorization phase with countdown timer. ESC shows quit confirmation. Help button (H) reveals grid for 1 second (5 uses max).

### Custom Mode Toggles

- Tutorial (on/off)
- Family path (on/off)
- Night school (on/off)
- Risky route (on/off)
- Investments (on/off)
- Pets (on/off)
- Spin to Win (on/off)
- Electronic banking theme (on/off)
- House sale spins (on/off)
- Retirement bonuses (on/off)

## Winning

Final worth = cash + house value + action cards + pet cards + baby bonuses + retirement bonus - loan penalties. Highest wins with ranked breakdown.

## Getting Started

### Quick Launch

```bash
# Clone the repository
git clone https://github.com/yourusername/Goldrush.git
cd Goldrush

# Install dependencies (see options below for your system)
# macOS:
brew install ncurses

# Ubuntu/Debian/WSL:
sudo apt-get install libncurses5-dev libncursesw5-dev

# Arch Linux:
sudo pacman -S ncurses

# Build and run the game
make run
```

The game will start immediately. Press `N` for new game, `L` to load a previous game, or `ESC` to quit.

## Requirements

- C++17 or later
- ncurses
- Unix-like terminal (including WSL on Windows)

Install ncurses:
- macOS: brew install ncurses
- Ubuntu/Debian/WSL: sudo apt-get install libncurses5-dev libncursesw5-dev
- Arch Linux: sudo pacman -S ncurses

## Build

make            # Build main game
make debug      # Build debug suite
make run        # Run main game
make run-debug  # Run debug suite
make clean      # Clean build files

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

## 1860 Movement Controls (during movement phase)

| Key | Action |
|-----|--------|
| Up / W | Move north (toward Retirement) |
| Right / D | Move east (toward Retirement) |
| Down / S | Move south (backward) |
| Left / A | Move west (backward) |
| Enter | Stop moving (keep remaining points) |
| ESC / Q | Cancel movement (undo if no steps taken) |

## Project Structure

├── main.cpp                     # Entry point
│
├── Game Core
│   ├── game.hpp/cpp             # Main game flow and logic
│   ├── game_settings.h/cpp      # Game settings management
│   ├── rules.hpp/cpp            # Game rules and toggles
│   └── player.hpp/cpp           # Player data structures
│
├── Board & Movement
│   ├── board.hpp/cpp            # Board data, rendering, view modes
│   ├── tile_display.h/cpp       # Tile display helpers
│   └── spins.hpp/cpp            # Spin result mappings
│
├── Cards & Decks
│   ├── cards.hpp/cpp            # Card systems (Action, Career, House, Invest, Pet)
│   └── deck.hpp                 # Deck management
│
├── Economy & Banking
│   ├── bank.hpp/cpp             # Banking and loan management
│   └── economy_manager.cpp      # Economy effects and actions
│
├── Sabotage System
│   ├── sabotage.h/cpp           # Sabotage system core
│   ├── sabotage_card.h/cpp      # Sabotage card definitions
│   └── cpuai.cpp                # CPU sabotage decisions
│
├── AI System
│   ├── cpu_player.hpp/cpp       # AI player decisions
│   └── random_service.hpp       # RNG with seed support
│
├── Minigames
│   ├── pong.hpp/cpp             # Pong minigame
│   ├── battleship.hpp/cpp       # Battleship minigame
│   ├── hangman.hpp/cpp          # Hangman minigame
│   ├── memory.hpp/cpp           # Memory Match minigame
│   ├── minesweeper.hpp/cpp      # Minesweeper minigame
│   └── minigame_manager.cpp     # Minigame orchestration
│
├── UI System
│   ├── ui.h/cpp                 # UI components and drawing
│   ├── ui_helpers.h/cpp         # UI helper functions
│   ├── ui_layout.h/cpp          # Layout calculations
│   └── ui_manager.cpp           # UI management and popups
│
├── Save & History
│   ├── save_manager.hpp/cpp     # Save/load functionality
│   ├── completed_history.h/cpp  # Completed game history tracking
│   ├── history.hpp/cpp          # In-game action history
│   └── history_manager.cpp      # History management
│
├── Tutorial System
│   ├── tutorials.h/cpp          # Tutorial system core
│   ├── tutorial_manager.cpp     # Tutorial triggering
│   └── minigame_tutorials.h/cpp # Minigame-specific tutorials
│
├── UI Components
│   ├── turn_summary.h/cpp       # Turn summary popups
│   ├── timer_display.h/cpp      # Countdown timer
│   └── input_helpers.h/cpp      # Input validation
│
├── Debug & Utilities
│   ├── debug.h/cpp              # Debug utilities
│   └── spins.cpp                # Spin result mappings (duplicate)
│
├── Build
│   ├── Makefile                 # Build configuration
│   └── .gitignore               # Git ignore rules
│
├── Data
│   ├── saves/                   # Save file directory
│   └── backups.tar.gz           # Backup archive
│
├── Documentation
│   └── README.md                # Project documentation
│
└── Executable
    └── gameoflife               # Compiled game binary

## Contributors

Cheralyn, Michelle, Yin, Carla, Joylin

## Notes

Educational project. The Game of Life is a trademark of Hasbro.

Enjoy! May the best life win! 🎉