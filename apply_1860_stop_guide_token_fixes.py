from pathlib import Path


def read(path):
    p = Path(path)
    if not p.exists():
        raise SystemExit(f"ERROR: {path} not found. Run this from the Goldrush project folder.")
    return p.read_text()


def write(path, text):
    p = Path(path)
    backup = p.with_suffix(p.suffix + ".bak_1860_updates")
    if not backup.exists():
        backup.write_text(p.read_text())
    p.write_text(text)
    print(f"[OK] updated {path} (backup: {backup.name})")


def replace_once(text, old, new, label, required=True):
    if old not in text:
        if required:
            raise SystemExit(f"ERROR: could not find target block for {label}")
        print(f"[SKIP] {label}")
        return text
    print(f"[OK] {label}")
    return text.replace(old, new, 1)


def find_function_span(text, func_name):
    idx = text.find(func_name)
    if idx == -1:
        raise SystemExit(f"ERROR: could not find function signature containing: {func_name}")

    brace = text.find("{", idx)
    if brace == -1:
        raise SystemExit(f"ERROR: could not find opening brace for: {func_name}")

    depth = 0
    for i in range(brace, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return idx, i + 1

    raise SystemExit(f"ERROR: could not find closing brace for: {func_name}")


def replace_function(path, func_name, new_function):
    text = read(path)
    start, end = find_function_span(text, func_name)
    text = text[:start] + new_function + text[end:]
    write(path, text)


# -----------------------------------------------------------------------------
# 1. Human 1860 movement:
#    - route planning still works
#    - STOP tile shows popup
#    - if STOP leaves movement remaining, planning resumes
#    - after STOP, title becomes exactly "1860 movement:"
# -----------------------------------------------------------------------------

new_move_human = r'''bool Game::moveHumanManually1860(int currentPlayer, int steps) {
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
}'''

replace_function("game.cpp", "bool Game::moveHumanManually1860", new_move_human)


# -----------------------------------------------------------------------------
# 2. After spinner popup:
#    "You have X movement. Use W/A/S/D..."
# -----------------------------------------------------------------------------

game = read("game.cpp")

old_take_spin_anchor = r'''    if (roll <= 0) {
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

    if (boardViewMode == BoardViewMode::Mode1860) {'''

new_take_spin_anchor = r'''    if (roll <= 0) {
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

    if (boardViewMode == BoardViewMode::Mode1860) {'''

if "Use W/A/S/D to move around. You can only move toward Retirement" not in game:
    game = replace_once(game, old_take_spin_anchor, new_take_spin_anchor, "add 1860 movement popup after spin")
else:
    print("[SKIP] 1860 movement popup after spin already exists")

write("game.cpp", game)


# -----------------------------------------------------------------------------
# 3. Hide 1860 quick-guide pages when choosing non-1860 board modes.
# -----------------------------------------------------------------------------

game = read("game.cpp")
game = replace_once(
    game,
    "showPreGameQuickGuide(hasColor);",
    "showPreGameQuickGuide(hasColor, boardViewMode == BoardViewMode::Mode1860);",
    "make quick guide conditional on board mode",
    required=False,
)
write("game.cpp", game)

header = read("tutorials.h")
header = replace_once(
    header,
    "void showPreGameQuickGuide(bool hasColor);",
    "void showPreGameQuickGuide(bool hasColor, bool include1860Pages = true);",
    "update quick guide declaration",
    required=False,
)
write("tutorials.h", header)

new_quick_guide = r'''void showPreGameQuickGuide(bool hasColor, bool include1860Pages) {
    std::vector<std::vector<std::string> > pages;
    pages.push_back({
        "MONEY AND LOANS",
        "",
        "Cash pays for choices, penalties, houses, and big events.",
        "If you cannot afford a payment, the game may automatically give you loans.",
        "Manual loans are emergency cash, but every loan reduces your final score."
    });

    if (include1860Pages) {
        pages.push_back({
            "1860 BOARD GOAL",
            "",
            "On the 1860 board, you begin near the bottom-left and work toward the top-right.",
            "Your goal is to reach Retirement in the top-right corner before your rivals do.",
            "Each spin determines how many movement points you can spend.",
            "Plan your route carefully, avoid risky spaces when possible, and use safer stops to protect your money."
        });
        pages.push_back({
            "1860 MOVEMENT",
            "",
            "A spin gives the maximum number of movement points you may spend that turn.",
            "Before the wagon moves, you can preview a route and pull the marker back along that uncommitted trail.",
            "Once you press ENTER, the route is committed and the 1860 road does not allow backward travel.",
            "You can stop before spending the full spin, so a big spin gives route choices instead of forcing every step."
        });
        pages.push_back({
            "1860 BOARD COLORS",
            "",
            "Pay/Safe spaces give steadier money and are usually the safer route.",
            "Action spaces trigger life or board events. Mini spaces lead to side challenges.",
            "Risk spaces can pay more, but they can also punish you badly.",
            "Job spaces shape salary progress, and Family spaces push family-related events.",
            "The symbols help too: S Start, R Retire, A Action, M Mini, ! Risk, + Safe, J Job, F Family."
        });
    }

    pages.push_back({
        "JOBS, SALARY, AND INVESTMENTS",
        "",
        "Jobs set your salary. Paydays add salary plus the space payout.",
        "College costs more early, but can unlock stronger careers.",
        "Investments pay when any spinner result matches your investment number."
    });
    pages.push_back({
        "LIFE EVENTS",
        "",
        "Marriage, babies, pets, and houses can change your money and final score.",
        "Kids and pets are part of your story and may create bonuses or costs.",
        "Houses can be sold near the end of the game."
    });
    pages.push_back({
        "CARDS, DEFENSE, AND MINIGAMES",
        "",
        "Action cards can help, hurt, move you, or start duels.",
        "Minigames award money and resolve some competitions.",
        "Insurance reduces some losses. Shields block harmful sabotage."
    });
    pages.push_back({
        "SABOTAGE AND ENDGAME",
        "",
        "Sabotage unlocks on each player's Turn 3.",
        "After that, players and CPUs can interfere with opponents.",
        "Final score counts cash, homes, pets, kids, cards,",
        "retirement bonuses, and loan penalties.",
        "Press G in-game to look at guide and tutorials again."
    });

    showPagedGuide("QUICK GUIDE", pages, hasColor);
}'''

replace_function("tutorials.cpp", "void showPreGameQuickGuide", new_quick_guide)


# -----------------------------------------------------------------------------
# 4. Custom character token on 1860 board:
#    Replace hardcoded P1/P2/P12 marker with selected player.token characters.
# -----------------------------------------------------------------------------

board = read("board.cpp")

old_marker_func = r'''std::string mode1860PlayerMarker(const std::vector<int>& occupants) {
    if (occupants.empty()) {
        return "";
    }

    if (occupants.size() == 1U) {
        return "P" + std::to_string(occupants.front() + 1);
    }

    std::string marker = "P";
    for (std::size_t i = 0; i < occupants.size(); ++i) {
        marker += std::to_string(occupants[i] + 1);
    }
    return clipText(marker, MODE1860_CELL_W - 1);
}'''

new_marker_func = r'''std::string mode1860PlayerMarker(const std::vector<Player>& players,
                                 const std::vector<int>& occupants) {
    if (occupants.empty()) {
        return "";
    }

    std::string marker;
    for (std::size_t i = 0; i < occupants.size(); ++i) {
        const int playerIndex = occupants[i];
        if (playerIndex >= 0 && playerIndex < static_cast<int>(players.size())) {
            marker.push_back(players[static_cast<std::size_t>(playerIndex)].token);
        }
    }

    if (marker.empty()) {
        return "?";
    }

    return clipText(marker, MODE1860_CELL_W - 1);
}'''

board = replace_once(
    board,
    old_marker_func,
    new_marker_func,
    "use custom tokens for 1860 marker",
    required=False,
)

board = replace_once(
    board,
    "std::string marker = mode1860PlayerMarker(occupants);",
    "std::string marker = mode1860PlayerMarker(players, occupants);",
    "update 1860 marker call",
    required=False,
)

write("board.cpp", board)

print("")
print("Done. Now run:")
print("  make")
