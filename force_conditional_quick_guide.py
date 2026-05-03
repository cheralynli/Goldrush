from pathlib import Path

path = Path("game.cpp")
if not path.exists():
    raise SystemExit("ERROR: game.cpp not found. Run this from the Goldrush project folder.")

text = path.read_text()

signature = "void Game::showTutorial()"
start = text.find(signature)

if start == -1:
    raise SystemExit("ERROR: Could not find void Game::showTutorial() in game.cpp.")

brace = text.find("{", start)
if brace == -1:
    raise SystemExit("ERROR: Could not find opening brace for Game::showTutorial().")

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
    raise SystemExit("ERROR: Could not find end of Game::showTutorial().")

new_func = '''void Game::showTutorial() {
    if (!rules.toggles.tutorialEnabled) {
        return;
    }

    showPreGameQuickGuide(hasColor, boardViewMode == BoardViewMode::Mode1860);
    addHistory("Quick guide reviewed");
}'''

backup = path.with_suffix(".cpp.bak_conditional_quick_guide")
if not backup.exists():
    backup.write_text(text)

path.write_text(text[:start] + new_func + text[end:])

print("[OK] Forced Game::showTutorial() to use board-mode-specific quick guide.")
print(f"[OK] Backup saved as {backup}")
