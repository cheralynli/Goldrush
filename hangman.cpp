#include "hangman.hpp"
#include "ui.h"

#include <ncurses.h>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

namespace {

const std::vector<std::string> WORD_LIST = {
    // Mining tools & equipment
    "PICKAXE", "SHOVEL", "PICK", "PAN", "SLUICE", "CRADLE", "ROCKER",
    "LONGBOX", "PUDDLER", "BATTERY", "STAMPER", "GOLD", "NUGGET",
    "FLASK", "SCALES", "CANTEEN", "LANTERN", "WHIPSAW", "WINDLASS",
    
    // Mining techniques
    "PANNING", "DREDGING", "SIFTING", "SLOUICING", "DRYDIGGING",
    "CREVICING", "QUARTZCRUSHING", "CABIN", "HUT", "TENT",
    
    // Gold measurements
    "OUNCE", "TROY", "GRAIN", "PENNYWEIGHT", "NUGGET", "DUST", "FLAKE",
    
    // Miner types
    "PROSPECTOR", "MINER", "FORTYMINER", "DIGGER", "FOSSICKER",
    "CLAIMJUMPER", "HARDBOOT", "BANJO", "QUACKER", "WHIMPERER",
    
    // Mining claims
    "CLAIM", "LODE", "VEIN", "REEF", "PLACER", "CREEK", "BEDROCK",
    "BONANZA", "STRIKE", "PAYDIRT", "LEDGE", "DRIFT", "SNAKE",
    
    // Gold Rush towns
    "COLOMA", "SUTTER", "CREEK", "SACRAMENTO", "DRYTOWN", "MARIPOSA",
    "SONORA", "ANGELSCAMP", "MOUNT", "SHASTA", "AUBURN", "NEVADA",
    
    // Miner life
    "GRUBSTAKE", "PROVISION", "OUTFIT", "MULE", "BURRO", "WAGON",
    "PACKTRAIN", "EMPORIUM", "GENERALSTORE", "SALOON", "GAMBLER",
    
    // Mining camps
    "CAMP", "SETTLEMENT", "DIGGINGS", "WORKINGS", "GULCH", "FLAT",
    "HILL", "BAR", "BENCH", "POINT", "RAVINE", "HOLLOW",
    
    // Problems
    "CAVIN", "FLOOD", "RIVER", "WATER", "ARGON", "SCORPION", 
    "RATTLESNAKE", "BANDIT", "HIGHWAYMAN", "SNAKE", "FLY", "CRICKET",
    
    // Gold Rush era
    "CALIFORNIA", "FORTYNINE", "SUTTERSMILL", "MARSHALL", "GOLD",
    "RUSH", "STAMPEDE", "HARDSHIP", "FRONTIER", "WILDWEST",
    
    // Transportation
    "STAGECOACH", "HORSE", "STEAMBOAT", "FERRY", "MULE", "WALK",
};

const std::vector<std::string> HANGMAN_STAGES = {
    "\n  +---+\n  |   |\n      |\n      |\n      |\n      |\n=========",
    "\n  +---+\n  |   |\n  O   |\n      |\n      |\n      |\n=========",
    "\n  +---+\n  |   |\n  O   |\n  |   |\n      |\n      |\n=========",
    "\n  +---+\n  |   |\n  O   |\n /|   |\n      |\n      |\n=========",
    "\n  +---+\n  |   |\n  O   |\n /|\\  |\n      |\n      |\n=========",
    "\n  +---+\n  |   |\n  O   |\n /|\\  |\n /    |\n      |\n=========",
    "\n  +---+\n  |   |\n  O   |\n /|\\  |\n / \\  |\n      |\n=========",
    "\n  +---+\n  |   |\n  O   |\n /|\\  |\n / \\  |\n      |\n========= GAME OVER!"
};

std::string getRandomWord() {
    static bool seeded = false;
    if (!seeded) {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        seeded = true;
    }
    return WORD_LIST[std::rand() % WORD_LIST.size()];
}

void drawHangman(WINDOW* win, int y, int x, int wrong, bool hasColor) {
    int stage = wrong > 7 ? 7 : wrong;
    std::string art = HANGMAN_STAGES[stage];
    
    if (hasColor) {
        wattron(win, COLOR_PAIR(GOLDRUSH_GOLD_TERRA) | A_BOLD);
    }
    
    int ly = y, lx = x;
    for (char c : art) {
        if (c == '\n') {
            ly++;
            lx = x;
        } else {
            mvwaddch(win, ly, lx++, c);
        }
    }
    
    if (hasColor) {
        wattroff(win, COLOR_PAIR(GOLDRUSH_GOLD_TERRA) | A_BOLD);
    }
}

void drawWord(WINDOW* win, int y, int x, const std::string& disp, bool hasColor) {
    if (hasColor) {
        wattron(win, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD | A_UNDERLINE);
    } else {
        wattron(win, A_BOLD | A_UNDERLINE);
    }
    
    for (size_t i = 0; i < disp.size(); i++) {
        mvwaddch(win, y, x + i * 2, disp[i]);
        mvwaddch(win, y, x + i * 2 + 1, ' ');
    }
    
    if (hasColor) {
        wattroff(win, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD | A_UNDERLINE);
    } else {
        wattroff(win, A_BOLD | A_UNDERLINE);
    }
}

void drawGuessed(WINDOW* win, int y, int x, const std::string& guessed, bool hasColor) {
    if (hasColor) {
        wattron(win, COLOR_PAIR(GOLDRUSH_BROWN_CREAM));
    }
    
    mvwprintw(win, y, x, "Guessed: ");
    for (int i = 0; i < 26; i++) {
        if (guessed[i] != ' ') {
            if (hasColor) {
                wattron(win, COLOR_PAIR(GOLDRUSH_BLACK_FOREST) | A_BOLD);
            }
            mvwaddch(win, y, x + 9 + i * 2, guessed[i]);
            mvwaddch(win, y, x + 9 + i * 2 + 1, ' ');
            if (hasColor) {
                wattroff(win, COLOR_PAIR(GOLDRUSH_BLACK_FOREST) | A_BOLD);
            }
        }
    }
    
    if (hasColor) {
        wattroff(win, COLOR_PAIR(GOLDRUSH_BROWN_CREAM));
    }
}

} // anonymous namespace

HangmanResult playHangmanMinigame(const std::string& playerName, bool hasColor) {
    HangmanResult res;
    res.won = false;
    res.attemptsLeft = 10;
    res.abandoned = false;

    int h, w;
    getmaxyx(stdscr, h, w);
    WINDOW* win = newwin(h, w, 0, 0);
    keypad(win, TRUE);

    // Set background color
    if (hasColor) {
        wbkgd(win, COLOR_PAIR(GOLDRUSH_GOLD_BLACK));
    }

    std::string word = getRandomWord();
    std::string disp(word.size(), '_');
    std::string guessed(26, ' ');
    int wrong = 0;

    while (true) {
        werase(win);
        
        // Title
        if (hasColor) {
            wattron(win, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);
        }
        mvwprintw(win, 1, (w - 16) / 2, "HANGMAN SIDEGAME");
        if (hasColor) {
            wattroff(win, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);
        }
        
        // Player info
        if (hasColor) {
            wattron(win, COLOR_PAIR(GOLDRUSH_BROWN_CREAM));
        }
        mvwprintw(win, 2, (w - 50) / 2, "Player: %s  |  Wrong guesses: %d/10", 
                  playerName.c_str(), wrong);
        if (hasColor) {
            wattroff(win, COLOR_PAIR(GOLDRUSH_BROWN_CREAM));
        }
        
        // Draw hangman
        drawHangman(win, 5, 5, wrong, hasColor);
        
        // Draw word display
        int wordX = (w - static_cast<int>(disp.size()) * 2) / 2;
        drawWord(win, 18, wordX, disp, hasColor);
        
        // Draw guessed letters
        drawGuessed(win, 20, 5, guessed, hasColor);
        
        // Instructions
        if (hasColor) {
            wattron(win, COLOR_PAIR(GOLDRUSH_BLACK_CREAM));
        }
        mvwprintw(win, 22, 5, "Press a letter (A-Z) to guess. Press Q to quit.");
        mvwprintw(win, 23, 5, "Attempts left: %d", 10 - wrong);
        if (hasColor) {
            wattroff(win, COLOR_PAIR(GOLDRUSH_BLACK_CREAM));
        }
        
        // Check win
        if (disp == word) {
            res.won = true;
            res.attemptsLeft = 10 - wrong;
            
            werase(win);
            if (hasColor) {
                wattron(win, COLOR_PAIR(GOLDRUSH_BLACK_FOREST) | A_BOLD);
            }
            mvwprintw(win, h/2 - 2, (w - 30) / 2, "CONGRATULATIONS!");
            mvwprintw(win, h/2 - 1, (w - 40) / 2, "You guessed the word: %s", word.c_str());
            mvwprintw(win, h/2, (w - 40) / 2, "Attempts left: %d", res.attemptsLeft);
            mvwprintw(win, h/2 + 2, (w - 30) / 2, "Press ENTER to continue.");
            if (hasColor) {
                wattroff(win, COLOR_PAIR(GOLDRUSH_BLACK_FOREST) | A_BOLD);
            }
            wrefresh(win);
            
            int ch;
            do {
                ch = wgetch(win);
            } while (ch != '\n' && ch != '\r' && ch != KEY_ENTER);
            break;
        }
        
        // Check loss
        if (wrong >= 10) {
            res.won = false;
            res.attemptsLeft = 0;
            
            werase(win);
            if (hasColor) {
                wattron(win, COLOR_PAIR(GOLDRUSH_GOLD_TERRA) | A_BOLD);
            }
            mvwprintw(win, h/2 - 2, (w - 30) / 2, "GAME OVER!");
            mvwprintw(win, h/2 - 1, (w - 40) / 2, "The word was: %s", word.c_str());
            mvwprintw(win, h/2 + 2, (w - 30) / 2, "Press ENTER to continue.");
            if (hasColor) {
                wattroff(win, COLOR_PAIR(GOLDRUSH_GOLD_TERRA) | A_BOLD);
            }
            wrefresh(win);
            
            int ch;
            do {
                ch = wgetch(win);
            } while (ch != '\n' && ch != '\r' && ch != KEY_ENTER);
            break;
        }

        wrefresh(win);

        // Get user input
        int ch = wgetch(win);
        
        if (ch == 'q' || ch == 'Q') {
            res.abandoned = true;
            break;
        }
        
        // Convert to uppercase
        if (ch >= 'a' && ch <= 'z') {
            ch = ch - 'a' + 'A';
        }
        
        if (ch >= 'A' && ch <= 'Z') {
            int idx = ch - 'A';
            
            // Check if already guessed
            if (guessed[idx] != ' ') {
                if (hasColor) {
                    wattron(win, COLOR_PAIR(GOLDRUSH_GOLD_TERRA));
                }
                mvwprintw(win, 24, 5, "Already guessed '%c'!", ch);
                if (hasColor) {
                    wattroff(win, COLOR_PAIR(GOLDRUSH_GOLD_TERRA));
                }
                wrefresh(win);
                napms(800);
                continue;
            }
            
            // Mark as guessed
            guessed[idx] = static_cast<char>(ch);
            
            // Check if letter is in word
            bool found = false;
            for (size_t i = 0; i < word.size(); i++) {
                if (word[i] == ch) {
                    disp[i] = static_cast<char>(ch);
                    found = true;
                }
            }
            
            if (!found) {
                wrong++;
            }
        }
    }

    delwin(win);
    touchwin(stdscr);
    refresh();
    
    return res;
}