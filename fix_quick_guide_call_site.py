from pathlib import Path
import re

path = Path("game.cpp")
if not path.exists():
    raise SystemExit("ERROR: game.cpp not found. Run this from the Goldrush project folder.")

text = path.read_text()

backup = path.with_suffix(".cpp.bak_quick_guide_call_site")
if not backup.exists():
    backup.write_text(text)

if "showPreGameQuickGuide(hasColor, boardViewMode == BoardViewMode::Mode1860);" in text:
    print("[OK] game.cpp already has board-mode-specific quick guide call.")
    raise SystemExit(0)

pattern = re.compile(
    r"(\n\s*setupInvestments\(\);\s*)\n\s*showTutorial\(\);\s*(\n\s*break;)"
)

replacement = r'''\1
            if (rules.toggles.tutorialEnabled) {
                showPreGameQuickGuide(hasColor, boardViewMode == BoardViewMode::Mode1860);
                addHistory("Quick guide reviewed");
            }\2'''

new_text, count = pattern.subn(replacement, text, count=1)

if count == 0:
    raise SystemExit(
        "ERROR: Could not find setupInvestments(); showTutorial(); break; block in game.cpp."
    )

path.write_text(new_text)

print("[OK] Replaced showTutorial() call with board-mode-specific quick guide logic.")
print(f"[OK] Backup saved as {backup}")
