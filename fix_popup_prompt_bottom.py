from pathlib import Path

path = Path("ui_helpers.cpp")

if not path.exists():
    raise SystemExit("ERROR: ui_helpers.cpp not found. Run this from the Goldrush project folder.")

text = path.read_text()

signature = """void showPopupMessage(const std::string& title,
                      const std::vector<std::string>& lines,
                      bool hasColor,
                      bool autoAdvance) {"""

start = text.find(signature)
if start == -1:
    raise SystemExit("ERROR: Could not find the vector version of showPopupMessage() in ui_helpers.cpp.")

brace_start = text.find("{", start)
if brace_start == -1:
    raise SystemExit("ERROR: Found showPopupMessage signature, but could not find opening brace.")

depth = 0
end = None

for i in range(brace_start, len(text)):
    if text[i] == "{":
        depth += 1
    elif text[i] == "}":
        depth -= 1
        if depth == 0:
            end = i + 1
            break

if end is None:
    raise SystemExit("ERROR: Could not find the end of showPopupMessage().")

new_function = r'''void showPopupMessage(const std::string& title,
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

    blinkIndicator(popup,
                   2,
                   3,
                   clipUiText(title, static_cast<std::size_t>(actualContentW)),
                   hasColor,
                   GOLDRUSH_GOLD_SAND,
                   2,
                   0,
                   actualContentW);

    const int bodyStartY = 5;

    // Keep the prompt on the very bottom usable row, matching the turn summary popup.
    const int promptY = std::max(1, actualH - 2);

    // Reserve one blank line above the prompt.
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

    // Clear the blank row and prompt row so old/body text cannot visually touch the prompt.
    if (blankRowY > 0) {
        mvwprintw(popup, blankRowY, 2, "%-*s", std::max(0, actualW - 4), "");
    }
    mvwprintw(popup, promptY, 2, "%-*s", std::max(0, actualW - 4), "");

    if (autoAdvance) {
        mvwprintw(popup,
                  promptY,
                  3,
                  "%s",
                  clipUiText("Continuing...",
                             static_cast<std::size_t>(actualContentW)).c_str());
        wrefresh(popup);
        napms(2000);
    } else {
        waitForEnterPrompt(popup,
                           promptY,
                           3,
                           clipUiText("Press ENTER or ESC to continue...",
                                      static_cast<std::size_t>(actualContentW)));
    }

    delwin(popup);
    wnoutrefresh(stdscr);
    doupdate();
}'''

backup = path.with_suffix(".cpp.bak_popup_prompt_bottom")
backup.write_text(text)

path.write_text(text[:start] + new_function + text[end:])

print("[OK] Updated ui_helpers.cpp")
print(f"[OK] Backup saved as {backup}")
print("")
print("Next run:")
print("  make")
