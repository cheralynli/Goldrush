#include <ncurses.h>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <cstring>

static const int MIN_W = 80;
static const int MIN_H = 24;

static const int TITLE_W = 76;
static const int TITLE_H = 3;
static const int BOARD_W = 76;
static const int BOARD_H = 14;
static const int INFO_W = 76;
static const int INFO_H = 4;
static const int MSG_W = 76;
static const int MSG_H = 3;

struct Player {
    std::string name;
    int node = 0;
    int cash = 10000;
    std::string job = "Unemployed";
    int salary = 0;
    bool married = false;
    int kids = 0;
    bool hasHouse = false;
    int houseValue = 0;
    bool retired = false;
    int choiceStart = -1; // 0 = College, 1 = Career
    int choiceFC = -1;    // 0 = Family, 1 = Career
    int choiceSR = -1;    // 0 = Safe, 1 = Risk
};

enum NodeId {
    START = 0,
    COLLEGE,
    CAREER,
    GRADUATION,
    WEDDING,
    BRANCH_FC,
    FAMILY_PATH,
    HOUSE,
    CAREER_PATH,
    PROMOTION,
    BRANCH_SR,
    SAFE_ROAD,
    RISK_ROAD,
    RETIREMENT,
    NODE_COUNT
};

struct Node {
    std::string name;
    int y;
    int x;
    int next;
    int altNext;
    bool isBranch;
};

static void drawHLine(WINDOW* w, int y, int x1, int x2) {
    for (int x = x1; x <= x2; ++x) mvwaddch(w, y, x, ACS_HLINE);
}

static void drawVLine(WINDOW* w, int x, int y1, int y2) {
    for (int y = y1; y <= y2; ++y) mvwaddch(w, y, x, ACS_VLINE);
}

static void waitForEnter(WINDOW* w, int y, int x, const std::string& msg) {
    mvwprintw(w, y, x, "%s", msg.c_str());
    wrefresh(w);
    flushinp();
    int ch;
    do {
        ch = wgetch(w);
    } while (ch != '\n' && ch != KEY_ENTER);
}

static void applyWindowBg(WINDOW* w, bool hasColor) {
    if (!w) return;
    if (hasColor) {
        wbkgd(w, COLOR_PAIR(5));
    }
}

static bool ensureMinSize(bool hasColor) {
    int h, w;
    while (true) {
        getmaxyx(stdscr, h, w);
        if (h >= MIN_H && w >= MIN_W) return true;

        if (hasColor) {
            bkgd(COLOR_PAIR(5));
        }
        clear();
        const char* msg1 = "Terminal too small - please resize";
        const char* msg2 = "Press Q to quit";
        int x1 = (w - static_cast<int>(std::strlen(msg1))) / 2;
        int x2 = (w - static_cast<int>(std::strlen(msg2))) / 2;
        int y = h / 2;
        if (x1 < 0) x1 = 0;
        if (x2 < 0) x2 = 0;
        mvprintw(y, x1, "%s", msg1);
        mvprintw(y + 1, x2, "%s", msg2);
        refresh();

        timeout(200);
        int ch = getch();
        if (ch == 'q' || ch == 'Q') return false;
        if (ch == KEY_RESIZE) {
            clear();
        }
    }
}

static void createWindows(int termH, int termW, WINDOW*& titleWin, WINDOW*& boardWin, WINDOW*& infoWin, WINDOW*& msgWin, bool hasColor) {
    int totalH = TITLE_H + BOARD_H + INFO_H + MSG_H;
    int startY = (termH - totalH) / 2;
    int startX = (termW - TITLE_W) / 2;
    if (startY < 0) startY = 0;
    if (startX < 0) startX = 0;

    titleWin = newwin(TITLE_H, TITLE_W, startY, startX);
    boardWin = newwin(BOARD_H, BOARD_W, startY + TITLE_H, startX);
    infoWin = newwin(INFO_H, INFO_W, startY + TITLE_H + BOARD_H, startX);
    msgWin = newwin(MSG_H, MSG_W, startY + TITLE_H + BOARD_H + INFO_H, startX);

    applyWindowBg(titleWin, hasColor);
    applyWindowBg(boardWin, hasColor);
    applyWindowBg(infoWin, hasColor);
    applyWindowBg(msgWin, hasColor);
}

static void destroyWindows(WINDOW*& titleWin, WINDOW*& boardWin, WINDOW*& infoWin, WINDOW*& msgWin) {
    if (titleWin) { delwin(titleWin); titleWin = nullptr; }
    if (boardWin) { delwin(boardWin); boardWin = nullptr; }
    if (infoWin) { delwin(infoWin); infoWin = nullptr; }
    if (msgWin) { delwin(msgWin); msgWin = nullptr; }
}

static int spin(WINDOW* w, bool hasColor) {
    werase(w);
    box(w, 0, 0);
    mvwprintw(w, 1, 2, "Hold SPACE to roll");
    mvwprintw(w, 2, 2, "Release to stop. Then ENTER to confirm");
    wrefresh(w);

    int ch;
    while (true) {
        ch = wgetch(w);
        if (ch == ' ') break;
    }

    nodelay(w, TRUE);
    int result = 1;
    auto lastInput = std::chrono::steady_clock::now();

    while (true) {
        result = (std::rand() % 10) + 1;
        werase(w);
        box(w, 0, 0);
        mvwprintw(w, 1, 2, "Rolling: %d", result);
        wrefresh(w);

        napms(80);
        ch = wgetch(w);
        if (ch == ' ') {
            lastInput = std::chrono::steady_clock::now();
        }
        auto now = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastInput).count();
        if (ms > 300) break;
    }
    nodelay(w, FALSE);

    for (int i = 0; i < 4; ++i) {
        werase(w);
        box(w, 0, 0);
        if (hasColor) wattron(w, COLOR_PAIR(6));
        mvwprintw(w, 1, 2, "Spin: %d", result);
        if (hasColor) wattroff(w, COLOR_PAIR(6));
        mvwprintw(w, 2, 2, "Press ENTER to confirm");
        wrefresh(w);
        napms(150);

        werase(w);
        box(w, 0, 0);
        mvwprintw(w, 1, 2, "Spin: %d", result);
        mvwprintw(w, 2, 2, "Press ENTER to confirm");
        wrefresh(w);
        napms(150);
    }

    int key;
    do {
        key = wgetch(w);
    } while (key != '\n' && key != KEY_ENTER);

    return result;
}

static int askChoice(WINDOW* w, const std::string& prompt, char a, char b) {
    werase(w);
    box(w, 0, 0);
    mvwprintw(w, 1, 2, "%s [%c/%c]", prompt.c_str(), a, b);
    wrefresh(w);
    int ch;
    while (true) {
        ch = wgetch(w);
        if (ch == a || ch == b) return ch;
        if (ch == a + 32 || ch == b + 32) return ch - 32;
    }
}

static void applyEffect(Player& p, int node, WINDOW* w) {
    werase(w);
    box(w, 0, 0);

    switch (node) {
        case START:
            mvwprintw(w, 1, 2, "START: Begin your journey.");
            break;
        case COLLEGE:
            p.cash -= 10000;
            p.cash += 20000;
            mvwprintw(w, 1, 2, "COLLEGE: -$10K +$20K loan.");
            break;
        case CAREER:
            p.job = "Clerk";
            p.salary = 3000;
            mvwprintw(w, 1, 2, "CAREER: Clerk ($3000 salary).");
            break;
        case GRADUATION:
            if (p.job == "Clerk") {
                p.job = "Manager";
                p.salary = 5000;
                mvwprintw(w, 1, 2, "GRADUATION: Manager ($5000).");
            } else {
                p.job = "Doctor";
                p.salary = 8000;
                mvwprintw(w, 1, 2, "GRADUATION: Doctor ($8000).");
            }
            break;
        case WEDDING:
            p.cash -= 5000;
            p.married = true;
            mvwprintw(w, 1, 2, "WEDDING: -$5000. Married = Yes.");
            break;
        case FAMILY_PATH:
            p.kids += 1;
            p.cash -= 2000;
            mvwprintw(w, 1, 2, "FAMILY PATH: +1 kid, -$2000.");
            break;
        case CAREER_PATH:
            p.salary += 5000;
            mvwprintw(w, 1, 2, "CAREER PATH: +$5000 salary.");
            break;
        case HOUSE:
            p.cash -= 50000;
            p.hasHouse = true;
            p.houseValue = 100000;
            mvwprintw(w, 1, 2, "HOUSE: -$50K, house value $100K.");
            break;
        case PROMOTION:
            p.salary += 5000;
            mvwprintw(w, 1, 2, "PROMOTION: +$5000 salary.");
            break;
        case SAFE_ROAD:
            p.cash += 3000;
            mvwprintw(w, 1, 2, "SAFE ROAD: +$3000.");
            break;
        case RISK_ROAD: {
            bool win = (std::rand() % 2) == 0;
            if (win) {
                p.cash += 15000;
                mvwprintw(w, 1, 2, "RISK ROAD: +$15000!");
            } else {
                p.cash -= 10000;
                mvwprintw(w, 1, 2, "RISK ROAD: -$10000.");
            }
            break;
        }
        case RETIREMENT:
            p.retired = true;
            mvwprintw(w, 1, 2, "RETIREMENT: You finished.");
            break;
        default:
            mvwprintw(w, 1, 2, "Nothing happens.");
            break;
    }

    wrefresh(w);
    waitForEnter(w, 2, 2, "Press ENTER to continue");
}

static int totalWorth(const Player& p) {
    int total = p.cash + (p.kids * 20000);
    if (p.hasHouse) total += p.houseValue;
    return total;
}

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    bool hasColor = has_colors();
    if (hasColor) {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);   // Player 1
        init_pair(2, COLOR_CYAN, COLOR_BLACK);    // Player 2
        init_pair(3, COLOR_MAGENTA, COLOR_BLACK); // Player 3
        init_pair(4, COLOR_YELLOW, COLOR_BLACK);  // Player 4
        init_pair(5, COLOR_WHITE, COLOR_BLACK);   // Default text
        init_pair(6, COLOR_RED, COLOR_BLACK);     // Risk / blink
        init_pair(7, COLOR_BLUE, COLOR_BLACK);    // Safe
        bkgd(COLOR_PAIR(5));
    }

    if (!ensureMinSize(hasColor)) {
        endwin();
        return 0;
    }

    WINDOW* titleWin = nullptr;
    WINDOW* boardWin = nullptr;
    WINDOW* infoWin = nullptr;
    WINDOW* msgWin = nullptr;
    int termH, termW;
    getmaxyx(stdscr, termH, termW);
    createWindows(termH, termW, titleWin, boardWin, infoWin, msgWin, hasColor);

    std::vector<Node> nodes(NODE_COUNT);
    nodes[START] = {"START", 2, 8, COLLEGE, CAREER, true};
    nodes[COLLEGE] = {"COLLEGE", 4, 8, GRADUATION, -1, false};
    nodes[CAREER] = {"CAREER", 4, 28, GRADUATION, -1, false};
    nodes[GRADUATION] = {"GRAD", 5, 18, WEDDING, -1, false};
    nodes[WEDDING] = {"WEDDING", 6, 18, BRANCH_FC, -1, false};
    nodes[BRANCH_FC] = {"BRANCH", 7, 18, FAMILY_PATH, CAREER_PATH, true};
    nodes[FAMILY_PATH] = {"FAMILY", 7, 10, HOUSE, -1, false};
    nodes[HOUSE] = {"HOUSE", 9, 10, BRANCH_SR, -1, false};
    nodes[CAREER_PATH] = {"CAREER", 7, 26, PROMOTION, -1, false};
    nodes[PROMOTION] = {"PROMO", 9, 26, BRANCH_SR, -1, false};
    nodes[BRANCH_SR] = {"BRANCH", 10, 18, SAFE_ROAD, RISK_ROAD, true};
    nodes[SAFE_ROAD] = {"SAFE", 10, 10, RETIREMENT, -1, false};
    nodes[RISK_ROAD] = {"RISK", 10, 26, RETIREMENT, -1, false};
    nodes[RETIREMENT] = {"RETIRE", 12, 18, -1, -1, false};

    // Title
    werase(titleWin);
    box(titleWin, 0, 0);
    if (hasColor) wattron(titleWin, COLOR_PAIR(5));
    mvwprintw(titleWin, 1, 22, "T H E   G A M E   O F   L I F E");
    if (hasColor) wattroff(titleWin, COLOR_PAIR(5));
    wrefresh(titleWin);

    // Start screen: ask player count and names
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

    std::vector<Player> players;
    players.reserve(numPlayers);
    for (int i = 0; i < numPlayers; ++i) {
        werase(msgWin);
        box(msgWin, 0, 0);
        mvwprintw(msgWin, 1, 2, "Player %d name: ", i + 1);
        wrefresh(msgWin);
        char namebuf[32] = {0};
        wgetnstr(msgWin, namebuf, 31);
        Player p;
        p.name = namebuf;
        players.push_back(p);
    }
    noecho();
    curs_set(0);

    bool allRetired = false;
    int current = 0;

    while (!allRetired) {
        if (!ensureMinSize(hasColor)) {
            destroyWindows(titleWin, boardWin, infoWin, msgWin);
            endwin();
            return 0;
        }
        getmaxyx(stdscr, termH, termW);
        destroyWindows(titleWin, boardWin, infoWin, msgWin);
        createWindows(termH, termW, titleWin, boardWin, infoWin, msgWin, hasColor);

        Player& p = players[current];
        if (p.retired) {
            current = (current + 1) % numPlayers;
            continue;
        }

    // Draw board
        werase(boardWin);
        box(boardWin, 0, 0);

        // Draw path lines
        drawVLine(boardWin, 6, 2, 4);
        drawVLine(boardWin, 32, 2, 4);
        drawHLine(boardWin, 4, 6, 32);
        drawVLine(boardWin, 19, 4, 7);
        drawVLine(boardWin, 9, 7, 9);
        drawVLine(boardWin, 29, 7, 9);
        drawHLine(boardWin, 7, 9, 29);
        drawVLine(boardWin, 19, 9, 10);
        drawHLine(boardWin, 10, 9, 29);

        mvwaddch(boardWin, 4, 6, ACS_LLCORNER);
        mvwaddch(boardWin, 4, 32, ACS_LRCORNER);
        mvwaddch(boardWin, 7, 19, ACS_PLUS);
        mvwaddch(boardWin, 10, 19, ACS_PLUS);

        // Labels
        for (int i = 0; i < NODE_COUNT; ++i) {
            int y = nodes[i].y;
            int x = nodes[i].x;
            // Draw tile box (3x9)
            mvwaddch(boardWin, y - 1, x - 2, ACS_ULCORNER);
            drawHLine(boardWin, y - 1, x - 1, x + 6);
            mvwaddch(boardWin, y - 1, x + 7, ACS_URCORNER);
            drawVLine(boardWin, x - 2, y, y + 1);
            drawVLine(boardWin, x + 7, y, y + 1);
            mvwaddch(boardWin, y + 2, x - 2, ACS_LLCORNER);
            drawHLine(boardWin, y + 2, x - 1, x + 6);
            mvwaddch(boardWin, y + 2, x + 7, ACS_LRCORNER);

            if (hasColor) {
                int color = 5;
                if (i == SAFE_ROAD) color = 7;
                if (i == RISK_ROAD) color = 6;
                wattron(boardWin, COLOR_PAIR(color));
                mvwprintw(boardWin, y, x, "%s", nodes[i].name.c_str());
                wattroff(boardWin, COLOR_PAIR(color));
            } else {
                mvwprintw(boardWin, y, x, "%s", nodes[i].name.c_str());
            }
        }

        // Tokens
        for (int i = 0; i < numPlayers; ++i) {
            int ny = nodes[players[i].node].y;
            int nx = nodes[players[i].node].x + 1;
            if (hasColor) wattron(boardWin, COLOR_PAIR(1 + (i % 4)));
            mvwaddch(boardWin, ny + 1, nx + (i % 4), '1' + i);
            if (hasColor) wattroff(boardWin, COLOR_PAIR(1 + (i % 4)));
        }

        wrefresh(boardWin);

        // Info panel (current player)
        werase(infoWin);
        box(infoWin, 0, 0);
        mvwprintw(infoWin, 1, 2, "PLAYER: %s", p.name.c_str());
        mvwprintw(infoWin, 2, 2, "Cash: $%d  Job: %s  Salary: $%d", p.cash, p.job.c_str(), p.salary);
        mvwprintw(infoWin, 3, 2, "Married: %s  Kids: %d", p.married ? "Yes" : "No", p.kids);
        mvwprintw(infoWin, 4, 2, "House: %s", p.hasHouse ? "Yes" : "No");
        mvwprintw(infoWin, 5, 2, "[ENTER] Spin  |  [Q] Quit");
        wrefresh(infoWin);

        // Wait for ENTER or Q
        int ch;
        do {
            ch = wgetch(infoWin);
            if (ch == 'q' || ch == 'Q') {
                endwin();
                return 0;
            }
        } while (ch != '\n' && ch != KEY_ENTER);

        int roll = spin(msgWin, hasColor);

        // Move steps without forcing branch choice mid-move.
        for (int step = 0; step < roll; ++step) {
            int n = p.node;
            if (n == START && p.choiceStart != -1) {
                p.node = (p.choiceStart == 0) ? nodes[n].next : nodes[n].altNext;
            } else if (n == BRANCH_FC && p.choiceFC != -1) {
                p.node = (p.choiceFC == 0) ? nodes[n].next : nodes[n].altNext;
            } else if (n == BRANCH_SR && p.choiceSR != -1) {
                p.node = (p.choiceSR == 0) ? nodes[n].next : nodes[n].altNext;
            } else {
                p.node = nodes[n].next;
            }
            if (p.node < 0) break;
        }

        // If landed on a branch node, ask for the next move.
        if (p.node == START) {
            int c = askChoice(msgWin, "Choose path: [A] College / [B] Career", 'A', 'B');
            p.choiceStart = (c == 'A') ? 0 : 1;
        } else if (p.node == BRANCH_FC) {
            int c = askChoice(msgWin, "Choose path: [A] Family / [B] Career", 'A', 'B');
            p.choiceFC = (c == 'A') ? 0 : 1;
        } else if (p.node == BRANCH_SR) {
            int c = askChoice(msgWin, "Choose road: [A] Safe / [B] Risk", 'A', 'B');
            p.choiceSR = (c == 'A') ? 0 : 1;
        }

        applyEffect(p, p.node, msgWin);

        // Check retirement
        allRetired = true;
        for (const auto& pl : players) {
            if (!pl.retired) {
                allRetired = false;
                break;
            }
        }

        current = (current + 1) % numPlayers;
    }

    // Game over
    werase(msgWin);
    box(msgWin, 0, 0);
    mvwprintw(msgWin, 1, 2, "GAME OVER!");

    int best = 0;
    int bestScore = totalWorth(players[0]);
    for (int i = 1; i < numPlayers; ++i) {
        int score = totalWorth(players[i]);
        if (score > bestScore) {
            bestScore = score;
            best = i;
        }
    }
    mvwprintw(msgWin, 2, 2, "%s wins! Total: $%d", players[best].name.c_str(), bestScore);
    wrefresh(msgWin);
    waitForEnter(msgWin, 2, 40, "Press ENTER to exit");

    destroyWindows(titleWin, boardWin, infoWin, msgWin);
    endwin();
    return 0;
}
