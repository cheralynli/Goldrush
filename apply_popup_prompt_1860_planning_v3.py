from pathlib import Path

def replace_exact(path, old, new, required=False):
    p = Path(path)
    if not p.exists():
        print(f"[SKIP] {path} not found")
        return False
    text = p.read_text()
    if old not in text:
        msg = "[MISS]" if required else "[SKIP]"
        print(f"{msg} {path}: target text not found")
        return False
    p.write_text(text.replace(old, new, 1))
    print(f"[OK] {path}")
    return True

def replace_all_in_existing(path, replacements):
    p = Path(path)
    if not p.exists():
        print(f"[SKIP] {path} not found")
        return
    text = p.read_text()
    changed = False
    for old, new in replacements:
        if old in text:
            text = text.replace(old, new)
            changed = True
    if changed:
        p.write_text(text)
        print(f"[OK] {path}")
    else:
        print(f"[SKIP] {path}: no matching story text found")

# ------------------------------------------------------------
# 1. game.cpp: replace human 1860 movement with route planning
# ------------------------------------------------------------

old_move_human_1860 = r'''bool Game::moveHumanManually1860(int currentPlayer, int steps) {
    Player& player = players[static_cast<std::size_t>(currentPlayer)];
    if (!board.isMode1860WalkableTile(player.tile)) {
        player.tile = board.mode1860StartTileId();
    }

    int remaining = std::max(0, steps);
    bool moved = false;
    bool effectAppliedOnCurrentTile = false;
    keypad(boardWin, TRUE);
    while (remaining > 0 && !player.retired) {
        const std::vector<int> adjacent = validAdjacent1860Tiles(player.tile);
        if (adjacent.empty()) {
            showInfoPopup("1860 Movement", "No legal adjacent 1860 spaces are available.");
            break;
        }

        board.render1860Selection(boardWin,
                                  players,
                                  currentPlayer,
                                  player.tile,
                                  adjacent,
                                  remaining,
                                  hasColor);
        const Tile& current = board.tileAt(player.tile);
        draw_sidebar_ui(infoWin, board, players, currentPlayer, history.recent(), rules);
        draw_message_ui(
            msgWin,
            "1860 movement: " + std::to_string(remaining) + " point" + (remaining == 1 ? "" : "s") + " left",
            "Current: Space " + std::to_string(player.tile) + " - " + getTileDisplayName(current) +
                " | Goal: move toward Retirement in the top-right. Arrows/WASD move, Enter stops, Esc/Q cancel or stop.");

        const int ch = wgetch(boardWin);
        if (isConfirmKey(ch)) {
            break;
        }
        if (ch == 27 || ch == 'q' || ch == 'Q') {
            if (!moved) {
                showInfoPopup("1860 Movement", "Movement cancelled before any step was taken.");
                return false;
            }
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

        const int nextTile = board.mode1860TileIdAt(current.mode1860Y + dRow, current.mode1860X + dCol);
        if (!isLegal1860Step(player.tile, nextTile)) {
            beep();
            continue;
        }

        player.tile = nextTile;
        moved = true;
        --remaining;
        effectAppliedOnCurrentTile = false;
        renderGame(currentPlayer,
                   player.name + " moved to " + getTileDisplayName(board.tileAt(player.tile)),
                   "1860 manual movement. Goal: move toward Retirement in the top-right.");
        napms(120);
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
}'''

new_move_human_1860 = r'''bool Game::moveHumanManually1860(int currentPlayer, int steps) {
    Player& player = players[static_cast<std::size_t>(currentPlayer)];
    if (!board.isMode1860WalkableTile(player.tile)) {
        player.tile = board.mode1860StartTileId();
    }

    const int startTile = player.tile;
    std::vector<int> plannedPath;
    plannedPath.push_back(startTile);

    int cursorTile = startTile;
    std::string statusTitle = "1860 route planning";
    std::string statusDetail =
        "Mark your trail first. ENTER travels it, BACKSPACE pulls the marker back, ESC cancels.";

    bool moved = false;
    bool effectAppliedOnCurrentTile = false;
    keypad(boardWin, TRUE);

    while (!player.retired) {
        const int plannedSteps = static_cast<int>(plannedPath.size()) - 1;
        const int remaining = std::max(0, steps - plannedSteps);

        std::vector<int> adjacent;
        if (remaining > 0) {
            adjacent = validAdjacent1860Tiles(cursorTile);
        }

        if (adjacent.empty() && plannedPath.size() == 1U && remaining > 0) {
            showInfoPopup("1860 Movement", "The trail has no legal forward space from here.");
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
                " | Planned " + std::to_string(plannedSteps) + "/" + std::to_string(steps) +
                ". " + statusDetail);

        const int ch = wgetch(boardWin);
        if (isConfirmKey(ch)) {
            break;
        }

        if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
            if (plannedPath.size() > 1U) {
                plannedPath.pop_back();
                cursorTile = plannedPath.back();
                statusTitle = "1860 route planning";
                statusDetail = "You pull the marker back along your own trail before committing.";
            } else {
                beep();
                statusTitle = "1860 route planning";
                statusDetail = "There is no planned trail to pull back yet.";
            }
            continue;
        }

        if (ch == 27 || ch == 'q' || ch == 'Q') {
            showInfoPopup("1860 Movement", "Route planning cancelled before the wagon moved.");
            return false;
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
            statusTitle = "1860 route planning";
            statusDetail = "You backtracked along the trail you have not committed yet.";
            continue;
        }

        if (remaining <= 0) {
            beep();
            statusTitle = "No movement points left";
            statusDetail = "Press ENTER to travel this planned route, or backtrack before committing.";
            continue;
        }

        if (!isLegal1860Step(cursorTile, nextTile)) {
            beep();
            statusTitle = "1860 rule: no backward travel";
            if (dRow > 0 || dCol < 0) {
                statusDetail =
                    "The 1860 road only goes forward toward Retirement. You may only undo your uncommitted trail.";
            } else {
                statusDetail = "That square is not part of the open forward trail.";
            }
            continue;
        }

        plannedPath.push_back(nextTile);
        cursorTile = nextTile;
        statusTitle = "1860 route planning";
        statusDetail = "Trail marked. Press ENTER to travel it, or step back along your trail before committing.";
    }

    if (plannedPath.size() <= 1U) {
        return false;
    }

    for (std::size_t step = 1; step < plannedPath.size() && !player.retired; ++step) {
        player.tile = plannedPath[step];
        moved = true;
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
            applyTileEffect(currentPlayer, board.tileAt(player.tile));
            effectAppliedOnCurrentTile = true;
        }
    }

    if (moved && !player.retired && !effectAppliedOnCurrentTile) {
        applyTileEffect(currentPlayer, board.tileAt(player.tile));
    }

    return moved;
}'''

replace_exact("game.cpp", old_move_human_1860, new_move_human_1860, required=True)

# ------------------------------------------------------------
# 2. Popup prompt newline fixes
# ------------------------------------------------------------

replace_exact(
    "battleship.cpp",
'''            mvwprintw(overlay, arenaBottom + 4, arenaLeft,
                      "Wave over. Destroyed %d/%d ships, payout $%d. Press ENTER or ESC.",
                      result.shipsDestroyed,
                      maxShips,
                      result.shipsDestroyed * 100);''',
'''            mvwprintw(overlay, arenaBottom + 4, arenaLeft,
                      "The smoke clears. You destroyed %d/%d ships and earned $%d.",
                      result.shipsDestroyed,
                      maxShips,
                      result.shipsDestroyed * 100);
            mvwprintw(overlay, arenaBottom + 6, arenaLeft,
                      "Press ENTER or ESC to continue.");'''
)

replace_exact(
    "pong.cpp",
'''            mvwprintw(overlay, arenaBottom + 4, arenaLeft,
                      "Game over. Final score %d, payout $%d. Press ENTER or ESC.",
                      result.playerScore,
                      result.playerScore * 100);''',
'''            mvwprintw(overlay, arenaBottom + 4, arenaLeft,
                      "The rally ends. Final score %d, payout $%d.",
                      result.playerScore,
                      result.playerScore * 100);
            mvwprintw(overlay, arenaBottom + 6, arenaLeft,
                      "Press ENTER or ESC to continue.");'''
)

replace_exact(
    "pong.cpp",
'''            const std::string endLine = std::string("Winner: ") +
                (result.winnerSide == 0 ? leftPlayerName : rightPlayerName) +
                ". Press ENTER or ESC.";
            mvwprintw(overlay, arenaBottom + 4, arenaLeft, "%s", endLine.c_str());''',
'''            const std::string endLine = std::string("The duel is settled. Winner: ") +
                (result.winnerSide == 0 ? leftPlayerName : rightPlayerName) +
                ".";
            mvwprintw(overlay, arenaBottom + 4, arenaLeft, "%s", endLine.c_str());
            mvwprintw(overlay, arenaBottom + 6, arenaLeft,
                      "Press ENTER or ESC to continue.");'''
)

replace_exact(
    "minesweeper.cpp",
'''            if (result.hitBomb) {
                endLine = "Bomb hit. Earned $" + std::to_string(result.safeTilesRevealed * 100) +
                          ". Press ENTER or ESC.";
            } else if (result.safeTilesRevealed >= TOTAL_SAFE_TILES) {
                endLine = "Board cleared. Earned $" + std::to_string(result.safeTilesRevealed * 100) +
                          ". Press ENTER or ESC.";
            } else {
                endLine = "Time up. Earned $" + std::to_string(result.safeTilesRevealed * 100) +
                          ". Press ENTER or ESC.";
            }
            mvwprintw(overlay, arenaBottom - 4,
                      arenaLeft + (arenaWidth - static_cast<int>(endLine.size())) / 2,
                      "%s", endLine.c_str());''',
'''            if (result.hitBomb) {
                endLine = "The ground gives way. Earned $" + std::to_string(result.safeTilesRevealed * 100) + ".";
            } else if (result.safeTilesRevealed >= TOTAL_SAFE_TILES) {
                endLine = "The field is cleared. Earned $" + std::to_string(result.safeTilesRevealed * 100) + ".";
            } else {
                endLine = "The clock runs out. Earned $" + std::to_string(result.safeTilesRevealed * 100) + ".";
            }
            mvwprintw(overlay, arenaBottom - 5,
                      arenaLeft + (arenaWidth - static_cast<int>(endLine.size())) / 2,
                      "%s", endLine.c_str());
            const std::string promptLine = "Press ENTER or ESC to continue.";
            mvwprintw(overlay, arenaBottom - 3,
                      arenaLeft + (arenaWidth - static_cast<int>(promptLine.size())) / 2,
                      "%s", promptLine.c_str());'''
)

replace_exact(
    "turn_summary.cpp",
'''    if (y + 1 >= getmaxy(win) - 2) {''',
'''    if (y + 1 >= getmaxy(win) - 4) {'''
)

replace_exact(
    "turn_summary.cpp",
'''                  clipUiText("\\nPress ENTER or ESC to continue...", static_cast<std::size_t>(std::max(1, actualW - 5))).c_str());''',
'''                  clipUiText("Press ENTER or ESC to continue...", static_cast<std::size_t>(std::max(1, actualW - 5))).c_str());'''
)

# ------------------------------------------------------------
# 3. Story-like minigame popup language wherever those strings live
# ------------------------------------------------------------

story_replacements = [
    ("One life. Each paddle return earns $100. Press ENTER to start.",
     "The saloon clears a table for Pong. Each return earns $100. Press ENTER to step in."),
    ("Shoot the $ ships. One enemy hit ends the run. Each ship is worth $100.",
     "Enemy ships cut across the river. Sink them for $100 each, but one enemy hit ends the run."),
    ("Guess the word. Hint unlocks after 5 misses. Each revealed letter is worth $100.",
     "A mystery word waits on the notice board. Reveal letters for $100 each; a hint appears after 5 misses."),
    ("Match all 8 pairs. Each wrong match costs a life. 20 lives total. Help button reveals grid.",
     "The cards are scattered across the table. Match all 8 pairs; wrong matches cost lives, and Help reveals the grid."),
    ("Reveal safe tiles for 60 seconds. One bomb ends the run. Each safe tile is worth $100.",
     "The claim is unstable. Reveal safe ground for 60 seconds; one bomb ends the run, and each safe tile pays $100.")
]

for path in Path(".").glob("*.cpp"):
    replace_all_in_existing(path, story_replacements)

# ------------------------------------------------------------
# 4. Tutorial text: explain 1860 cannot go backwards after ENTER
# ------------------------------------------------------------

replace_exact(
    "tutorials.cpp",
'''        "A spin gives the maximum number of movement points you may spend that turn.",
        "You can move up to what you spun, not necessarily the full amount.",
        "If you land on a stop or event tile before using every point, its effect happens first and then you may keep moving.",
        "So a big spin gives more options, and you can keep spending points until they run out or you choose to stop."''',
'''        "A spin gives the maximum number of movement points you may spend that turn.",
        "Before the wagon moves, you can preview a route and pull the marker back along that uncommitted trail.",
        "Once you press ENTER, the route is committed and the 1860 road does not allow backward travel.",
        "You can stop before spending the full spin, so a big spin gives route choices instead of forcing every step."'''
)

replace_exact(
    "tutorials.cpp",
'''        "On the 1860 board, you start at the bottom-left and work toward Retirement in the top-right.",
        "A spin gives the maximum movement points you may spend on that turn.",
        "You may stop before spending them all, and landing on a stop/event tile only pauses for its effect before movement can continue.",
        "That means a big spin gives you more route choices instead of forcing a hard stop halfway through."''',
'''        "On the 1860 board, you start at the bottom-left and work toward Retirement in the top-right.",
        "A spin gives the maximum movement points you may spend on that turn.",
        "During route planning, move the marker forward, backtrack along your uncommitted trail if needed, then press ENTER.",
        "After ENTER, the 1860 road is one-way: you cannot travel backward, only onward toward Retirement.",
        "That means a big spin gives you more route choices without forcing every point to be spent."'''
)

print("")
print("Done. Now run:")
print("  make")
print("or:")
print("  g++ -std=c++11 *.cpp -lncurses -o goldrush")
