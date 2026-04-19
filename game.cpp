#include "Game.hpp"
#include "ui.h"

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>

namespace {
enum MiniGameKind {
    MINIGAME_RED_BLACK,
    MINIGAME_MATH,
    MINIGAME_ODD_EVEN
};
}

Game::Game()
    : titleWin(nullptr),
      boardWin(nullptr),
      infoWin(nullptr),
      msgWin(nullptr),
      hasColor(has_colors()) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

Game::~Game() {
    destroyWindows();
}

void Game::applyWindowBg(WINDOW* w) const {
    apply_ui_background(w);
}

bool Game::ensureMinSize() const {
    int h, w;
    timeout(200);
    while (true) {
        getmaxyx(stdscr, h, w);
        if (h >= MIN_H && w >= MIN_W) {
            timeout(-1);
            return true;
        }

        if (hasColor) {
            bkgd(COLOR_PAIR(GOLDRUSH_GOLD_BLACK));
        }
        clear();
        const char* line1 = "Terminal too small - please resize";
        const char* line2 = "Minimum size: 116x40";
        const char* line3 = "Press Q to quit";
        int x1 = (w - static_cast<int>(std::strlen(line1))) / 2;
        int x2 = (w - static_cast<int>(std::strlen(line2))) / 2;
        int x3 = (w - static_cast<int>(std::strlen(line3))) / 2;
        int y = h / 2;
        if (x1 < 0) x1 = 0;
        if (x2 < 0) x2 = 0;
        if (x3 < 0) x3 = 0;
        mvprintw(y - 1, x1, "%s", line1);
        mvprintw(y, x2, "%s", line2);
        mvprintw(y + 1, x3, "%s", line3);
        refresh();

        int ch = getch();
        if (ch == 'q' || ch == 'Q') {
            timeout(-1);
            return false;
        }
    }
}

void Game::destroyWindows() {
    if (titleWin) { delwin(titleWin); titleWin = nullptr; }
    if (boardWin) { delwin(boardWin); boardWin = nullptr; }
    if (infoWin) { delwin(infoWin); infoWin = nullptr; }
    if (msgWin) { delwin(msgWin); msgWin = nullptr; }
}

void Game::createWindows() {
    int termH, termW;
    getmaxyx(stdscr, termH, termW);
    int totalH = TITLE_H + BOARD_H + MSG_H;
    int startY = (termH - totalH) / 2;
    int startX = (termW - TITLE_W) / 2;
    if (startY < 0) startY = 0;
    if (startX < 0) startX = 0;

    titleWin = newwin(TITLE_H, TITLE_W, startY, startX);
    boardWin = newwin(BOARD_H, BOARD_W, startY + TITLE_H, startX);
    infoWin = newwin(INFO_H, INFO_W, startY + TITLE_H, startX + BOARD_W);
    msgWin = newwin(MSG_H, MSG_W, startY + TITLE_H + BOARD_H, startX);

    applyWindowBg(titleWin);
    applyWindowBg(boardWin);
    applyWindowBg(infoWin);
    applyWindowBg(msgWin);
}

void Game::waitForEnter(WINDOW* w, int y, int x, const std::string& text) const {
    mvwprintw(w, y, x, "%s", text.c_str());
    wrefresh(w);
    int ch;
    do {
        ch = wgetch(w);
    } while (ch != '\n' && ch != KEY_ENTER);
}

bool Game::showStartScreen() {
    while (true) {
        int h, w;
        getmaxyx(stdscr, h, w);
        clear();
        if (hasColor) bkgd(COLOR_PAIR(GOLDRUSH_GOLD_BLACK));

        const char* lines[] = {
            "  ________       .__       .___                   .__     ",
            " /  _____/  ____ |  |    __| _/______ __ __  _____|  |__  ",
            "/   \\  ___ /  _ \\|  |   / __ |\\_  __ \\  |  \\/  ___/  |  \\ ",
            "\\    \\_\\  (  <_> )  |__/ /_/ | |  | \\/  |  /\\___ \\|   Y  \\",
            " \\______  /\\____/|____/\\____ | |__|  |____//____  >___|  /",
            "        \\/                  \\/                  \\/     \\/ "
        };

        int artW = 60;
        int startY = (h / 2) - 6;
        int startX = (w - artW) / 2;
        if (startY < 1) startY = 1;
        if (startX < 0) startX = 0;

        if (hasColor) attron(COLOR_PAIR(GOLDRUSH_GOLD_BLACK));
        mvprintw(startY - 1, startX - 4, "╔══════════════════════════════════════════════════════════════════╗");
        mvprintw(startY + 6, startX - 4, "║                                                                  ║");
        mvprintw(startY + 7, startX - 4, "╚══════════════════════════════════════════════════════════════════╝");
        if (hasColor) attroff(COLOR_PAIR(GOLDRUSH_GOLD_BLACK));

        if (hasColor) wattron(stdscr, COLOR_PAIR(GOLDRUSH_GOLD_BLACK) | A_BOLD);
        for (int i = 0; i < 6; ++i) {
            mvprintw(startY + i, startX, "%s", lines[i]);
        }
        if (hasColor) wattroff(stdscr, COLOR_PAIR(GOLDRUSH_GOLD_BLACK) | A_BOLD);

        if (hasColor) wattron(stdscr, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);
        mvprintw(startY + 9, (w - 8) / 2, "GOLDRUSH");
        if (hasColor) wattroff(stdscr, COLOR_PAIR(GOLDRUSH_GOLD_SAND) | A_BOLD);

        if (hasColor) wattron(stdscr, COLOR_PAIR(GOLDRUSH_BROWN_SAND));
        mvprintw(startY + 11, (w - 30) / 2, "An Adventure in Text");
        mvprintw(startY + 13, (w - 20) / 2, "S  Start    Q  Quit");
        if (hasColor) wattroff(stdscr, COLOR_PAIR(GOLDRUSH_BROWN_SAND));
        refresh();

        int ch = getch();
        if (ch == 's' || ch == 'S') return true;
        if (ch == 'q' || ch == 'Q') return false;
        if (ch == KEY_RESIZE && !ensureMinSize()) return false;
    }
}

void Game::setupPlayers() {
    echo();
    curs_set(1);
    int numPlayers = 0;
    while (numPlayers < 2 || numPlayers > 4) {
        werase(msgWin);
        box(msgWin, 0, 0);
        mvwprintw(msgWin, 1, 2, "How many players? (2-4): ");
        wrefresh(msgWin);
        char buf[8] = {0};
        wgetnstr(msgWin, buf, 7);
        numPlayers = std::atoi(buf);
    }

    players.clear();
    players.reserve(numPlayers);
    for (int i = 0; i < numPlayers; ++i) {
        werase(msgWin);
        box(msgWin, 0, 0);
        mvwprintw(msgWin, 1, 2, "Player %d name: ", i + 1);
        wrefresh(msgWin);
        char nameBuf[32] = {0};
        wgetnstr(msgWin, nameBuf, 31);

        Player p;
        p.name = nameBuf;
        p.token = tokenForName(p.name, i);
        p.tile = 0;
        p.cash = 10000;
        p.job = "Unemployed";
        p.salary = 0;
        p.married = false;
        p.kids = 0;
        p.hasHouse = false;
        p.houseValue = 0;
        p.retired = false;
        p.startChoice = -1;
        p.familyChoice = -1;
        players.push_back(p);
    }

    noecho();
    curs_set(0);
}

void Game::renderHeader() const {
    draw_title_banner_ui(titleWin);
}

void Game::renderGame(int currentPlayer, const std::string& msg) const {
    renderHeader();
    draw_board_ui(boardWin, board, players, players[currentPlayer].tile);

    const Player& p = players[currentPlayer];
    draw_right_panel_ui(infoWin, p, currentPlayer);
    draw_message_ui(msgWin, msg, "");
}

int Game::minRewardForTier(int tier) const {
    if (tier == 2) return 3000;
    if (tier >= 3) return 5000;
    return 1000;
}

int Game::maxRewardForTier(int tier) const {
    if (tier == 2) return 5000;
    if (tier >= 3) return 10000;
    return 2000;
}

int Game::rollSpinner() {
    werase(msgWin);
    box(msgWin, 0, 0);
    mvwprintw(msgWin, 1, 2, "Hold SPACE to roll. Release to stop.");
    mvwprintw(msgWin, 2, 2, "Result will blink. Press ENTER to confirm.");
    wrefresh(msgWin);

    int ch;
    do {
        ch = wgetch(msgWin);
    } while (ch != ' ');

    nodelay(msgWin, TRUE);
    auto lastSpace = std::chrono::steady_clock::now();
    int value = 1;
    while (true) {
        value = (std::rand() % 10) + 1;
        werase(msgWin);
        box(msgWin, 0, 0);
        mvwprintw(msgWin, 1, 2, "Rolling: %d", value);
        mvwprintw(msgWin, 2, 2, "Release SPACE to stop");
        wrefresh(msgWin);
        napms(80);

        ch = wgetch(msgWin);
        if (ch == ' ') {
            lastSpace = std::chrono::steady_clock::now();
        }
        long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - lastSpace).count();
        if (elapsed > 240) break;
    }
    nodelay(msgWin, FALSE);

    for (int i = 0; i < 4; ++i) {
        werase(msgWin);
        box(msgWin, 0, 0);
        if (hasColor) wattron(msgWin, COLOR_PAIR(GOLDRUSH_BLACK_GOLD));
        mvwprintw(msgWin, 1, 2, "Rolled: %d", value);
        if (hasColor) wattroff(msgWin, COLOR_PAIR(GOLDRUSH_BLACK_GOLD));
        mvwprintw(msgWin, 2, 2, "Press ENTER to confirm");
        wrefresh(msgWin);
        napms(140);

        werase(msgWin);
        box(msgWin, 0, 0);
        mvwprintw(msgWin, 1, 2, "Rolled: %d", value);
        mvwprintw(msgWin, 2, 2, "Press ENTER to confirm");
        wrefresh(msgWin);
        napms(140);
    }

    waitForEnter(msgWin, 2, 26, "");
    return value;
}

int Game::showBranchPopup(const std::string& title,
                          const std::vector<std::string>& lines,
                          char a,
                          char b) {
    (void)a;
    (void)b;
    std::vector<int> values;
    values.push_back(0);
    values.push_back(1);
    return choose_branch_with_selector(title, lines, values, 0);
}

int Game::playActionCard(const Tile& tile, Player& player) {
    int h, w;
    getmaxyx(stdscr, h, w);
    WINDOW* popup = newwin(9, 34, (h - 9) / 2, (w - 34) / 2);
    applyWindowBg(popup);

    int minAmount = minRewardForTier(tile.value);
    int maxAmount = maxRewardForTier(tile.value);
    int amount = minAmount + (std::rand() % (maxAmount - minAmount + 1));
    MiniGameKind game = static_cast<MiniGameKind>(std::rand() % 3);
    int ch;

    if (game == MINIGAME_RED_BLACK) {
        werase(popup);
        box(popup, 0, 0);
        mvwprintw(popup, 1, 2, "BLACK TILE ACTION");
        mvwprintw(popup, 2, 2, "Mini Game: Red / Black");
        mvwprintw(popup, 4, 2, "Pick [R]ed or [B]lack");
        mvwprintw(popup, 6, 2, "Win or lose: $%d", amount);
        wrefresh(popup);

        char guess = 'R';
        while (true) {
            ch = wgetch(popup);
            if (ch == 'r' || ch == 'R') { guess = 'R'; break; }
            if (ch == 'b' || ch == 'B') { guess = 'B'; break; }
        }

        char result = (std::rand() % 2 == 0) ? 'R' : 'B';
        bool win = (guess == result);
        if (win) player.cash += amount;
        else player.cash -= amount;

        werase(popup);
        box(popup, 0, 0);
        if (hasColor) wattron(popup, COLOR_PAIR(win ? GOLDRUSH_GOLD_FOREST : GOLDRUSH_BLACK_TERRA));
        mvwprintw(popup, 1, 2, "The wheel landed on %s!", result == 'R' ? "RED" : "BLACK");
        mvwprintw(popup, 3, 2, "You %s $%d", win ? "WIN" : "LOSE", amount);
        if (hasColor) wattroff(popup, COLOR_PAIR(win ? GOLDRUSH_GOLD_FOREST : GOLDRUSH_BLACK_TERRA));
        mvwprintw(popup, 5, 2, "New cash: $%d", player.cash);
        mvwprintw(popup, 6, 2, "Press ENTER");
        wrefresh(popup);
    } else if (game == MINIGAME_MATH) {
        int a = 2 + (std::rand() % 9);
        int b = 1 + (std::rand() % 9);
        bool add = (std::rand() % 2 == 0);
        int answer = add ? (a + b) : (a - b);
        if (!add && a < b) {
            int t = a;
            a = b;
            b = t;
            answer = a - b;
        }

        echo();
        curs_set(1);
        werase(popup);
        box(popup, 0, 0);
        mvwprintw(popup, 1, 2, "BLACK TILE ACTION");
        mvwprintw(popup, 2, 2, "Mini Game: Quick Math");
        mvwprintw(popup, 4, 2, "Solve: %d %c %d = ", a, add ? '+' : '-', b);
        mvwprintw(popup, 6, 2, "Win or lose: $%d", amount);
        wrefresh(popup);

        char buf[16] = {0};
        wgetnstr(popup, buf, 15);
        noecho();
        curs_set(0);
        int guess = std::atoi(buf);
        bool win = (guess == answer);
        if (win) player.cash += amount;
        else player.cash -= amount;

        werase(popup);
        box(popup, 0, 0);
        if (hasColor) wattron(popup, COLOR_PAIR(win ? GOLDRUSH_GOLD_FOREST : GOLDRUSH_BLACK_TERRA));
        mvwprintw(popup, 1, 2, "Correct answer: %d", answer);
        mvwprintw(popup, 3, 2, "You %s $%d", win ? "WIN" : "LOSE", amount);
        if (hasColor) wattroff(popup, COLOR_PAIR(win ? GOLDRUSH_GOLD_FOREST : GOLDRUSH_BLACK_TERRA));
        mvwprintw(popup, 5, 2, "New cash: $%d", player.cash);
        mvwprintw(popup, 6, 2, "Press ENTER");
        wrefresh(popup);
    } else {
        werase(popup);
        box(popup, 0, 0);
        mvwprintw(popup, 1, 2, "BLACK TILE ACTION");
        mvwprintw(popup, 2, 2, "Mini Game: Odd / Even");
        mvwprintw(popup, 4, 2, "Pick [O]dd or [E]ven");
        mvwprintw(popup, 6, 2, "Win or lose: $%d", amount);
        wrefresh(popup);

        char guess = 'O';
        while (true) {
            ch = wgetch(popup);
            if (ch == 'o' || ch == 'O') { guess = 'O'; break; }
            if (ch == 'e' || ch == 'E') { guess = 'E'; break; }
        }

        int spin = 1 + (std::rand() % 10);
        bool even = (spin % 2 == 0);
        bool win = (guess == 'E' && even) || (guess == 'O' && !even);
        if (win) player.cash += amount;
        else player.cash -= amount;

        werase(popup);
        box(popup, 0, 0);
        if (hasColor) wattron(popup, COLOR_PAIR(win ? GOLDRUSH_GOLD_FOREST : GOLDRUSH_BLACK_TERRA));
        mvwprintw(popup, 1, 2, "Spin: %d", spin);
        mvwprintw(popup, 2, 2, "%s number!", even ? "Even" : "Odd");
        mvwprintw(popup, 3, 2, "You %s $%d", win ? "WIN" : "LOSE", amount);
        if (hasColor) wattroff(popup, COLOR_PAIR(win ? GOLDRUSH_GOLD_FOREST : GOLDRUSH_BLACK_TERRA));
        mvwprintw(popup, 5, 2, "New cash: $%d", player.cash);
        mvwprintw(popup, 6, 2, "Press ENTER");
        wrefresh(popup);
    }

    do {
        ch = wgetch(popup);
    } while (ch != '\n' && ch != KEY_ENTER);

    delwin(popup);
    touchwin(msgWin);
    wrefresh(msgWin);
    return amount;
}

void Game::applyTileEffect(Player& player, const Tile& tile) {
    std::string line = "Keep moving.";

    switch (tile.kind) {
        case TILE_START:
            line = "START: The journey begins.";
            break;
        case TILE_BLACK:
            playActionCard(tile, player);
            return;
        case TILE_COLLEGE:
            player.cash += 10000;
            line = "COLLEGE: -$10K tuition, +$20K loan.";
            break;
        case TILE_CAREER:
            player.job = "Clerk";
            player.salary = 3000;
            line = "CAREER: You became a Clerk ($3000).";
            break;
        case TILE_GRADUATION:
            if (player.startChoice == 0) {
                player.job = "Doctor";
                player.salary = 8000;
                line = "GRADUATION: Doctor path unlocked. Salary is now $8000.";
            } else {
                player.job = "Manager";
                player.salary = 5000;
                line = "GRADUATION: Career path pays off. Salary is now $5000.";
            }
            break;
        case TILE_MARRIAGE:
            player.cash -= 5000;
            player.married = true;
            line = "MARRIAGE: -$5000 and married.";
            break;
        case TILE_CAREER_2:
            player.salary += tile.value;
            line = "CAREER PATH: promotion! Salary increased.";
            break;
        case TILE_PAYDAY:
            player.cash += tile.value;
            line = "PAYDAY: cash increased.";
            break;
        case TILE_BABY:
            player.kids += tile.value;
            player.cash -= tile.value * 1000;
            line = "FAMILY ROAD: babies added to the family.";
            break;
        case TILE_HOUSE:
            player.cash -= 50000;
            player.hasHouse = true;
            player.houseValue = tile.value;
            line = "HOUSE: bought a house on the family road.";
            break;
        case TILE_RETIREMENT:
            if (tile.id == 88) {
                player.retired = true;
                line = "RETIREMENT: You finished the game.";
            } else {
                line = "Retirement stretch.";
            }
            break;
        case TILE_SPLIT_START:
        case TILE_SPLIT_FAMILY:
        case TILE_EMPTY:
        case TILE_FAMILY:
        default:
            break;
    }

    werase(msgWin);
    box(msgWin, 0, 0);
    mvwprintw(msgWin, 1, 2, "%s", line.c_str());
    mvwprintw(msgWin, 2, 2, "Press ENTER to continue");
    wrefresh(msgWin);
    int ch;
    do {
        ch = wgetch(msgWin);
    } while (ch != '\n' && ch != KEY_ENTER);
}

int Game::chooseNextTile(Player& player, const Tile& tile) {
    if (tile.kind == TILE_SPLIT_START && player.startChoice == -1) {
        int c = showBranchPopup(
            "College or Career?",
            std::vector<std::string>{
                "- College: debt now, stronger graduation payoff",
                "- Career: income sooner, steadier path"
            },
            'A',
            'B');
        player.startChoice = c;
    }
    if (tile.kind == TILE_SPLIT_FAMILY && tile.id == 58 && player.familyChoice == -1) {
        int c = showBranchPopup(
            "Family or Career?",
            std::vector<std::string>{
                "- Family: babies, house chances, more chaos",
                "- Career: more payday tiles and promotions"
            },
            'A',
            'B');
        player.familyChoice = c;
    }

    if (tile.kind == TILE_SPLIT_START) {
        return (player.startChoice == 0) ? tile.next : tile.altNext;
    }
    if (tile.kind == TILE_SPLIT_FAMILY && tile.id == 58) {
        return (player.familyChoice == 0) ? tile.next : tile.altNext;
    }
    return tile.next;
}

void Game::animateMove(int currentPlayer, int steps) {
    Player& player = players[currentPlayer];
    for (int step = 0; step < steps; ++step) {
        const Tile& current = board.tileAt(player.tile);
        int nextTile = chooseNextTile(player, current);
        if (nextTile < 0) break;
        player.tile = nextTile;
        renderGame(currentPlayer, player.name + " moved to tile " + std::to_string(player.tile));
        napms(170);
    }
}

bool Game::run() {
    if (!ensureMinSize()) return false;
    if (!showStartScreen()) return false;

    createWindows();
    setupPlayers();

    int currentPlayer = 0;
    bool allRetired = false;
    while (!allRetired) {
        if (!ensureMinSize()) return false;
        destroyWindows();
        createWindows();

        if (players[currentPlayer].retired) {
            currentPlayer = (currentPlayer + 1) % static_cast<int>(players.size());
            allRetired = true;
            for (size_t i = 0; i < players.size(); ++i) {
                if (!players[i].retired) allRetired = false;
            }
            continue;
        }

        renderGame(currentPlayer, players[currentPlayer].name + "'s turn");

        int ch;
        do {
            ch = wgetch(infoWin);
            if (ch == 'q' || ch == 'Q') return false;
        } while (ch != '\n' && ch != KEY_ENTER);

        int roll = rollSpinner();
        animateMove(currentPlayer, roll);
        applyTileEffect(players[currentPlayer], board.tileAt(players[currentPlayer].tile));

        allRetired = true;
        for (size_t i = 0; i < players.size(); ++i) {
            if (!players[i].retired) {
                allRetired = false;
                break;
            }
        }
        currentPlayer = (currentPlayer + 1) % static_cast<int>(players.size());
    }

    int winner = 0;
    int best = totalWorth(players[0]);
    for (size_t i = 1; i < players.size(); ++i) {
        int worth = totalWorth(players[i]);
        if (worth > best) {
            best = worth;
            winner = static_cast<int>(i);
        }
    }

    werase(msgWin);
    box(msgWin, 0, 0);
    mvwprintw(msgWin, 1, 2, "Game Over! %s wins with $%d.", players[winner].name.c_str(), best);
    mvwprintw(msgWin, 2, 2, "Press ENTER to exit");
    wrefresh(msgWin);
    waitForEnter(msgWin, 2, 22, "");
    return true;
}
