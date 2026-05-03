from pathlib import Path

path = Path("ui_helpers.cpp")
text = path.read_text()

signature = '''void showPopupMessage(const std::string& title,
                      const std::vector<std::string>& lines,
                      bool hasColor,
                      bool autoAdvance) {'''

start = text.find(signature)
if start == -1:
    raise SystemExit("Could not find vector showPopupMessage() in ui_helpers.cpp")

brace = text.find("{", start)
depth = 0
end = None

for i in range(brace, len(text)):
    if text[i] == "{":
        depth += 1
    elif text[i] == "}":
        depth -= 1
        if depth == 0:
            end = i + 1
            break

if end is None:
    raise SystemExit("Could not find end of showPopupMessage()")

new_func = r'''void showPopupMessage(const std::string& title,
                      const std::vector<std::string>& lines,
                      bool hasColor,
                      bool autoAdvance) {
    int h = 0;
    int w = 0;
    getmaxyx(stdscr, h, w);

    const int popupW = std::min(std::max(58, w - 8), 86);
    const int estimatedContentW = std::max(1, popupW - 6);

    std::vector<std::string> wrapped;
    for (std::size_t i = 0; i < lines.size(); ++i) {
        const std::vector<std::string> part =
            wrapUiText(lines[i], static_cast<std::size_t>(estimatedContentW));
        wrapped.insert(wrapped.end(), part.begin(), part.end());
    }

    const int desiredH = std::max(15, static_cast<int>(wrapped.size()) + 12);
    const int popupHRequest = std::min(desiredH, std::max(8, h - 2));

    WINDOW* popup = createCenteredWindow(popupHRequest, popupW, 8, 32);
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
    const int promptY = actualH - 2;

    // Reserve one completely blank line before the prompt.
    const int bodyEndY = promptY - 2;
    const int maxBodyLines = std::max(0, bodyEndY - bodyStartY + 1);

    for (int i = 0; i < maxBodyLines && i < static_cast<int>(wrapped.size()); ++i) {
        mvwprintw(popup,
                  bodyStartY + i,
                  3,
                  "%s",
                  clipUiText(wrapped[static_cast<std::size_t>(i)],
                             static_cast<std::size_t>(actualContentW)).c_str());
    }

    if (static_cast<int>(wrapped.size()) > maxBodyLines && maxBodyLines > 0) {
        mvwprintw(popup,
                  bodyStartY + maxBodyLines - 1,
                  3,
                  "%s",
                  clipUiText("...more text omitted...",
                             static_cast<std::size_t>(actualContentW)).c_str());
    }

    if (promptY - 1 > 0) {
        mvwprintw(popup, promptY - 1, 2, "%-*s", std::max(0, actualW - 4), "");
    }

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

path.write_text(text[:start] + new_func + text[end:])
print("[OK] Replaced showPopupMessage() in ui_helpers.cpp")
