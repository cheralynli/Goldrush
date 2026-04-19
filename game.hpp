#pragma once

#include <ncurses.h>
#include <string>
#include <vector>

#include "Board.hpp"
#include "Player.hpp"

class Game {
public:
    Game();
    ~Game();

    bool run();

private:
    static const int MIN_W = 116;
    static const int MIN_H = 40;
    static const int TITLE_W = 114;
    static const int TITLE_H = 8;
    static const int BOARD_W = 82;
    static const int BOARD_H = 28;
    static const int INFO_W = 32;
    static const int INFO_H = 30;
    static const int MSG_W = 114;
    static const int MSG_H = 4;

    Board board;
    std::vector<Player> players;
    WINDOW* titleWin;
    WINDOW* boardWin;
    WINDOW* infoWin;
    WINDOW* msgWin;
    bool hasColor;

    bool ensureMinSize() const;
    void createWindows();
    void destroyWindows();
    void waitForEnter(WINDOW* w, int y, int x, const std::string& text) const;
    void applyWindowBg(WINDOW* w) const;

    bool showStartScreen();
    void setupPlayers();
    void renderGame(int currentPlayer, const std::string& msg) const;
    void renderHeader() const;
    int rollSpinner();
    int showBranchPopup(const std::string& title,
                        const std::vector<std::string>& lines,
                        char a,
                        char b);
    int playActionCard(const Tile& tile, Player& player);
    void applyTileEffect(Player& player, const Tile& tile);
    int chooseNextTile(Player& player, const Tile& tile);
    void animateMove(int currentPlayer, int steps);

    int minRewardForTier(int tier) const;
    int maxRewardForTier(int tier) const;
};
