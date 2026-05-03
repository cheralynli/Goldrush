from pathlib import Path

path = Path("ui.cpp")
if not path.exists():
    raise SystemExit("ERROR: ui.cpp not found. Run this from the Goldrush project folder.")

text = path.read_text()

backup = path.with_suffix(".cpp.bak_minimap_retire_tokens")
if not backup.exists():
    backup.write_text(text)

def replace_once(old, new, label, required=True):
    global text
    if old not in text:
        if required:
            raise SystemExit(f"ERROR: could not find target for {label}")
        print(f"[SKIP] {label}")
        return
    text = text.replace(old, new, 1)
    print(f"[OK] {label}")

# ---------------------------------------------------------------------
# Add helper for custom minimap player glyphs.
# ---------------------------------------------------------------------
helper_anchor = '''void drawMinimapDot(WINDOW* panelWin, int y, int x, const char* glyph, int colorPair) {
    wattron(panelWin, COLOR_PAIR(colorPair) | A_BOLD);
    mvwprintw(panelWin, y, x, "%s", glyph);
    wattroff(panelWin, COLOR_PAIR(colorPair) | A_BOLD);
}
'''

helper_replacement = helper_anchor + r'''
std::string minimapPlayerGlyph(const std::vector<Player>& players, int playerIndex) {
    if (playerIndex < 0 || playerIndex >= static_cast<int>(players.size())) {
        return "?";
    }

    char token = players[static_cast<std::size_t>(playerIndex)].token;
    if (token == '\0' || std::isspace(static_cast<unsigned char>(token))) {
        return "?";
    }

    return std::string(1, token);
}

int minimapRepresentativePlayerOnTile(const std::vector<Player>& players,
                                      int tileId,
                                      int currentPlayer) {
    if (currentPlayer >= 0 &&
        currentPlayer < static_cast<int>(players.size()) &&
        players[static_cast<std::size_t>(currentPlayer)].tile == tileId) {
        return currentPlayer;
    }

    for (int p = 0; p < static_cast<int>(players.size()); ++p) {
        if (players[static_cast<std::size_t>(p)].tile == tileId) {
            return p;
        }
    }

    return -1;
}
'''
replace_once(helper_anchor, helper_replacement, "add minimap custom token helpers")

# ---------------------------------------------------------------------
# 1860 minimap: retirement tile green instead of terra/red.
# ---------------------------------------------------------------------
replace_once(
'''                if (tile.kind == TILE_RETIREMENT) {
                    color = GOLDRUSH_GOLD_TERRA;
                } else if (isSpecialMinimapTile(tile)) {
                    color = GOLDRUSH_BLACK_TERRA;
                }
''',
'''                if (tile.kind == TILE_RETIREMENT) {
                    color = GOLDRUSH_BLACK_FOREST;
                } else if (isSpecialMinimapTile(tile)) {
                    color = GOLDRUSH_BLACK_TERRA;
                }
''',
"make 1860 retirement minimap tile green"
)

# ---------------------------------------------------------------------
# 1860 minimap: player marker uses custom token instead of "o".
# ---------------------------------------------------------------------
replace_once(
'''            drawMinimapDot(panelWin,
                        drawY,
                        drawX,
                        "o",
                        ui_player_color_pair(p));
''',
'''            const std::string playerGlyph = minimapPlayerGlyph(players, p);
            drawMinimapDot(panelWin,
                        drawY,
                        drawX,
                        playerGlyph.c_str(),
                        ui_player_color_pair(p));
''',
"make 1860 minimap use custom player token"
)

# ---------------------------------------------------------------------
# 1860 minimap legend.
# ---------------------------------------------------------------------
replace_once(
'''            mvwprintw(panelWin, mapBottom + 3, 2, ". tile  + special  * retire S start  o player");''',
'''            mvwprintw(panelWin, mapBottom + 3, 2, ". tile  + special  * retire S start  token player");''',
"update 1860 minimap legend"
)

# ---------------------------------------------------------------------
# Follow Camera minimap: use custom token for one or more occupants.
# For multiple players on one tile, prefer the current player's token if present.
# ---------------------------------------------------------------------
replace_once(
'''            if (occupants > 1) {
                drawMinimapDot(panelWin, drawY, drawX, "@", GOLDRUSH_GOLD_TERRA);
            } else if (occupants == 1) {
                drawMinimapDot(panelWin, drawY, drawX, "o", ui_player_color_pair(firstPlayer));
            } else {
                drawMinimapDot(panelWin,
                               drawY,
                               drawX,
                               minimapDotGlyph(tile),
                               tile.kind == TILE_RETIREMENT
                                   ? GOLDRUSH_GOLD_TERRA
                                   : (isSpecialMinimapTile(tile)
                                          ? GOLDRUSH_BLACK_TERRA
                                          : GOLDRUSH_BROWN_CREAM));
            }
''',
'''            if (occupants > 0) {
                const int shownPlayer = minimapRepresentativePlayerOnTile(players, i, currentPlayer);
                const std::string playerGlyph = minimapPlayerGlyph(players, shownPlayer);
                drawMinimapDot(panelWin,
                               drawY,
                               drawX,
                               playerGlyph.c_str(),
                               ui_player_color_pair(shownPlayer));
            } else {
                drawMinimapDot(panelWin,
                               drawY,
                               drawX,
                               minimapDotGlyph(tile),
                               tile.kind == TILE_RETIREMENT
                                   ? GOLDRUSH_BLACK_FOREST
                                   : (isSpecialMinimapTile(tile)
                                          ? GOLDRUSH_BLACK_TERRA
                                          : GOLDRUSH_BROWN_CREAM));
            }
''',
"make follow-camera minimap use custom tokens and green retire"
)

# ---------------------------------------------------------------------
# Follow Camera minimap legend.
# ---------------------------------------------------------------------
replace_once(
'''            mvwprintw(panelWin, mapBottom + 3, 2, ". tile  + special  * retire  S start  o player");''',
'''            mvwprintw(panelWin, mapBottom + 3, 2, ". tile  + special  * retire  S start  token player");''',
"update follow-camera minimap legend"
)

path.write_text(text)

print("")
print("[OK] ui.cpp patched.")
print(f"[OK] Backup saved as {backup}")
print("")
print("Next run:")
print("  make")
