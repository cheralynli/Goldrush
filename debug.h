#pragma once

//Purpose: entry point for the debug system
//Relation: provides access to all debug functions via a menu interface
void runDebugMenu();

void debugDiceRoll();
//Purpose: test dice/spinner randomness
//Relation: verifies RandomService and roll condition logic
void debugActionCards();
//Purpose: test action card drawing and resolution
//Relation: validates DeckManager and ActionCard effects
void debugPlayerMovement();
//Purpose: test player movement across tiles
//Relation: checks board navigation and stop-space logic
void debugSaveSystem();
//Purpose: test saving game state
//Relation: ensures persistence of decks, players, and rules
void debugLoadSystem();
//Purpose: test loading game state
//Relation: ensures restoration of decks, players, and rules
void debugCPUDecision();
//Purpose: test CPU decision-making logic
//Relation: validates CpuController route, career, and sabotage choices
void debugMinigames();
//Purpose: test minigame execution
//Relation: validates Pong, Battleship, Hangman, Memory, Minesweeper integration
void debugSabotage();
//Purpose: test sabotage mechanics
//Relation: validates sabotage targeting and effect resolution
void debugUiPacing();
//Purpose: test UI timing and responsiveness
//Relation: ensures ncurses pacing and feedback consistency
void debugBoardUi();
//Purpose: test board rendering.
//Relation: validates Board::render and tile drawing
void debugTileColorsAndSymbols();
//Purpose: test tile color and symbol rendering
//Relation: ensures ncurses color pairs and tile markers display correctly
void debugTileFullNameDisplay();
//Purpose: test tile label rendering
//Relation: ensures full names are displayed properly
void debugPlayerTokenHighlighting();
//Purpose: test player token highlighting
//Relation: validates focus and selection indicators
void debugBoardLegend();
//Purpose: test legend rendering
//Relation: ensures tutorial legend displays correctly
void debugBoardRegions();
//Purpose: test region mapping
//Relation: validates BoardRegion definitions
void debugBoardLandmarks();
//Purpose: test landmark rendering
//Relation: ensures special tiles (college, career, marriage, etc.) display correctly
void debugPlayerSidePanel();
//Purpose: test side panel UI
//Relation: validates player stats display
void debugHistoryFormatting();
//Purpose: test completed game history formatting
//Relation: validates CompletedGameEntry display
void debugCurrentObjectiveBox();
//Purpose: test objective box rendering
//Relation: ensures current goals are displayed correctly
void debugEventMessagePanel();
//Purpose: test event message rendering
//Relation: validates feedback and event notifications

