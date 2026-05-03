from pathlib import Path


def require_file(path):
    p = Path(path)
    if not p.exists():
        raise SystemExit(f"ERROR: {path} not found. Run this from the Goldrush project folder.")
    return p


def backup_once(path, suffix):
    p = Path(path)
    backup = p.with_suffix(p.suffix + suffix)
    if not backup.exists():
        backup.write_text(p.read_text())
    return backup


def find_function_span(text, signature_start):
    start = text.find(signature_start)
    if start == -1:
        raise SystemExit(f"ERROR: could not find function starting with:\n{signature_start}")

    brace = text.find("{", start)
    if brace == -1:
        raise SystemExit("ERROR: found function signature but not opening brace.")

    depth = 0
    for i in range(brace, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return start, i + 1

    raise SystemExit("ERROR: could not find function closing brace.")


def replace_function(path, signature_start, new_function, suffix):
    p = require_file(path)
    text = p.read_text()
    start, end = find_function_span(text, signature_start)
    backup = backup_once(p, suffix)
    p.write_text(text[:start] + new_function + text[end:])
    print(f"[OK] replaced function in {path}")
    print(f"[OK] backup: {backup.name}")


def replace_once(path, old, new, label, required=False):
    p = require_file(path)
    text = p.read_text()
    if old not in text:
        if required:
            raise SystemExit(f"ERROR: target not found for {label}")
        print(f"[SKIP] {label}")
        return False
    backup = backup_once(p, ".bak_popup_title_other_guide")
    p.write_text(text.replace(old, new, 1))
    print(f"[OK] {label}")
    print(f"[OK] backup: {backup.name}")
    return True


# -----------------------------------------------------------------------------
# 1. Fix showPopupMessage title blinking:
#    The blinking effect is now applied on the title row itself.
# -----------------------------------------------------------------------------

new_show_popup_message = r'''void showPopupMessage(const std::string& title,
                      const std::vector<std::string>& lines,
                      bool hasColor,
                      bool autoAdvance) {
    int screenH = 0;
    int screenW = 0;
    getmaxyx(stdscr, screenH, screenW);

    const int popupW = std::min(std::max(58, screenW - 8), 86);
    const int estimatedContentW = std::max(1, popupW - 6);

    std::vector<std::string> wrapped;
    for (std::size_t i = 0; i < lines.size(); ++i) {
        const std::vector<std::string> part =
            wrapUiText(lines[i], static_cast<std::size_t>(estimatedContentW));
        wrapped.insert(wrapped.end(), part.begin(), part.end());
    }

    const int preferredH = std::max(15, static_cast<int>(wrapped.size()) + 11);
    const int popupH = std::min(preferredH, std::max(8, screenH - 2));

    WINDOW* popup = createCenteredWindow(popupH, popupW, 8, 32);
    if (!popup) {
        showTerminalSizeWarning(8, 32, hasColor, !autoAdvance);
        return;
    }

    int actualH = 0;
    int actualW = 0;
    getmaxyx(popup, actualH, actualW);

    const int actualContentW = std::max(1, actualW - 6);

    wrapped.clear();
    for (std::size_t i = 0; i < lines.size(); ++i) {
        const std::vector<std::string> part =
            wrapUiText(lines[i], static_cast<std::size_t>(actualContentW));
        wrapped.insert(wrapped.end(), part.begin(), part.end());
    }

    keypad(popup, TRUE);
    werase(popup);
    drawBoxSafe(popup);

    // Title row: blinking is applied directly here, not one row below it.
    const int titleY = 1;
    const int titleX = 3;
    const std::string titleText =
        clipUiText(title, static_cast<std::size_t>(actualContentW));

    blinkIndicator(popup,
                   titleY,
                   titleX,
                   titleText,
                   hasColor,
                   GOLDRUSH_GOLD_SAND,
                   2,
                   0,
                   actualContentW);

    const int bodyStartY = 4;

    // Bottom usable row, same visual placement as the turn summary popup.
    const int promptY = std::max(1, actualH - 2);

    // Keep prompt safely inside the border and prevent wrapping.
    const int promptX = std::min(5, std::max(1, actualW - 2));

    // Keep one blank row above the prompt.
    const int blankRowY = promptY - 1;
    const int maxBodyLines = std::max(0, blankRowY - bodyStartY);

    const bool truncated = static_cast<int>(wrapped.size()) > maxBodyLines;
    const int linesToDraw =
        truncated && maxBodyLines > 0 ? maxBodyLines - 1 : maxBodyLines;

    for (int i = 0; i < linesToDraw && i < static_cast<int>(wrapped.size()); ++i) {
        mvwprintw(popup,
                  bodyStartY + i,
                  3,
                  "%s",
                  clipUiText(wrapped[static_cast<std::size_t>(i)],
                             static_cast<std::size_t>(actualContentW)).c_str());
    }

    if (truncated && maxBodyLines > 0) {
        mvwprintw(popup,
                  bodyStartY + maxBodyLines - 1,
                  3,
                  "%s",
                  clipUiText("...more text omitted...",
                             static_cast<std::size_t>(actualContentW)).c_str());
    }

    // Clear interior rows without touching the border.
    if (blankRowY > 0) {
        for (int x = 1; x < actualW - 1; ++x) {
            mvwaddch(popup, blankRowY, x, ' ');
        }
    }
    for (int x = 1; x < actualW - 1; ++x) {
        mvwaddch(popup, promptY, x, ' ');
    }

    const std::string promptText = autoAdvance
        ? "Continuing..."
        : "Press ENTER or ESC to continue...";

    // Use addnstr instead of printw so the prompt cannot wrap.
    const int maxPromptWidth = std::max(1, actualW - promptX - 2);
    const std::string clippedPrompt =
        clipUiText(promptText, static_cast<std::size_t>(maxPromptWidth));

    mvwaddnstr(popup,
               promptY,
               promptX,
               clippedPrompt.c_str(),
               maxPromptWidth);

    wrefresh(popup);

    if (autoAdvance) {
        napms(2000);
    } else {
        nodelay(popup, FALSE);
        waitForConfirmOrCancel(popup);
    }

    delwin(popup);
    wnoutrefresh(stdscr);
    doupdate();
}'''

replace_function(
    "ui_helpers.cpp",
    """void showPopupMessage(const std::string& title,
                      const std::vector<std::string>& lines,
                      bool hasColor,
                      bool autoAdvance) {""",
    new_show_popup_message,
    ".bak_popup_title_row"
)


# -----------------------------------------------------------------------------
# 2. Add other-mode abbreviation guide before game starts.
#
# Previous script made this call:
#   showPreGameQuickGuide(hasColor, boardViewMode == BoardViewMode::Mode1860);
#
# This script updates showPreGameQuickGuide() so when include1860Pages is false,
# it adds a normal-board abbreviation guide page.
# -----------------------------------------------------------------------------

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
    } else {
        pages.push_back({
            "NORMAL BOARD TILE GUIDE",
            "",
            "The other board modes use abbreviations on the route. Read them as Full Name (ABBR).",
            "START / ST: Start - the journey begins.",
            "CO: College - pay tuition and aim for stronger careers.",
            "CA: Career - choose a job and salary early.",
            "GR: Graduation - college players choose a degree career.",
            "PAY / SP: Salary Payday - collect salary and space payout.",
            "ACT / A: Action - draw or resolve an action card.",
            "MG / M: Minigame - play a side game for money.",
            "JOB / J: Job or career event - career progress happens here."
        });
        pages.push_back({
            "NORMAL BOARD LIFE TILES",
            "",
            "MAR: Marriage - resolve wedding gifts and family bonuses.",
            "BABY / B: Baby - spin for children.",
            "FAM / F: Family - family-path events may happen.",
            "NS: Night School - optional career upgrade.",
            "SAFE / +: Safe route - smaller but steadier payout.",
            "RISK / !: Risky route - bigger swings, good or bad.",
            "HOUSE / H: House - draw or buy a home.",
            "RET / R: Retirement - choose retirement destination and final bonus."
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

replace_function(
    "tutorials.cpp",
    "void showPreGameQuickGuide",
    new_quick_guide,
    ".bak_other_mode_guide"
)

# Make sure the declaration supports the bool argument.
replace_once(
    "tutorials.h",
    "void showPreGameQuickGuide(bool hasColor);",
    "void showPreGameQuickGuide(bool hasColor, bool include1860Pages = true);",
    "tutorials.h bool overload",
    required=False
)

# Make sure game.cpp passes the selected board mode to the quick guide.
# If it already does, this skips harmlessly.
replace_once(
    "game.cpp",
    "showPreGameQuickGuide(hasColor);",
    "showPreGameQuickGuide(hasColor, boardViewMode == BoardViewMode::Mode1860);",
    "game.cpp conditional quick guide",
    required=False
)

print("")
print("Done. Now run:")
print("  make")
