#pragma once

#include <ncurses.h>

#include <string>
#include <vector>

//Input: none 
//Output: holds UiWindowBounds attributes (height width, y, x)
//Purpose: represents the dimensions and bounds of the terminal
//Relation: accessed in calculating bounds to display popup windows
struct UiWindowBounds {
    int height;
    int width;
    int y;
    int x;
};

//Input: int minHeight, int minWidth
//Output: bool terminalIsAtLeast
//Purpose: check if terminal is correctly sized
//Relation: used in minigames to ensure game can be displayed properly
bool terminalIsAtLeast(int minHeight, int minWidth);

//Input: int minHeight, int minWidth, bool hasColor, bool waitForKey
//Output: none
//Purpose: display warning (terminal too small), and required/current terminal sizes
//Relation: used in minigames to ensure game can be displayed properly
void showTerminalSizeWarning(int minHeight, int minWidth, bool hasColor, bool waitForKey = true);

//Input: int preferredHeight, int preferredWidth, int minHeight, (int minWidth, UiWindowBounds bounds)
//Output: bool calculateCenteredWindowBounds
//Purpose: computes UiWindowBounds bounds based on preferences and constraints and creates a Centered Window
//Relation: used to createCenteredWindow
bool calculateCenteredWindowBounds(int preferredHeight,
                                   int preferredWidth,
                                   int minHeight,
                                   int minWidth,
                                   UiWindowBounds& bounds);
WINDOW* createCenteredWindow(int preferredHeight,
                             int preferredWidth,
                             int minHeight = 3,
                             int minWidth = 12);
void clearWindowInterior(WINDOW* win);

//Input: WINDOW win, int y(columns), int x(rows), string message
//Output: none
//Purpose: wait for user to enter prompt
//Relation: used in showPopupMessage while waiting for prompt
void waitForEnterPrompt(WINDOW* win,
                        int y,
                        int x,
                        const std::string& message = "Press ENTER to continue...");

//Input: string text, int blinkCount, int finalHoldMs
//Output: blinkIndicator()
//Purpose: simplify blinkIndicator()
//Relation: convenience wrapper for blinkIndicator()      
void blinkIndicator(const std::string& text,
                    int blinkCount = 2,
                    int finalHoldMs = 2000);

//Input: WINDOW win, int y(columns), int x(rows), string text, bool hasColor, int colorPair, int blinkCount, int finalHoldMs, int clearWidth
//Output: none
//Purpose: draw blinking text that blinks blinkCount times
//Relation: used in game.cpp prompts        
void blinkIndicator(WINDOW* win,
                    int y,
                    int x,
                    const std::string& text,
                    bool hasColor,
                    int colorPair,
                    int blinkCount = 2,
                    int finalHoldMs = 2000,
                    int clearWidth = 0);

//Input: string title, string message, bool hasColor, bool autoAdvance
//Output: showPopupMessage()
//Purpose: splits message into a vector of strings
//Relation: pre-processing of message before entering showPopupMessage
void showPopupMessage(const std::string& title,
                      const std::string& message,
                      bool hasColor,
                      bool autoAdvance = false);

//Input: string title, vector lines, bool hasColor, bool autoAdvance
//Output: none
//Purpose: create a window to display popup message
//Relation: used in showDecisionPopup and in game.cpp
void showPopupMessage(const std::string& title,
                      const std::vector<std::string>& lines,
                      bool hasColor,
                      bool autoAdvance = false);

//Input: string playerName, string decision, string explanation
//Output: showPopupMessage()
//Purpose: pop up a decision window
//Relation: used in game.cpp
void showDecisionPopup(const std::string& playerName,
                       const std::string& decision,
                       const std::string& explanation,
                       bool hasColor,
                       bool autoAdvance = false);

//Input: string text, size_t width
//Output: string / vector<string>
//Purpose: clip or wrap text
//Relation: used in showPopupMessage to ensure text fits in popup windows
std::string clipUiText(const std::string& text, std::size_t width);
std::vector<std::string> wrapUiText(const std::string& text, std::size_t width);
