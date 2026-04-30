#pragma once

#include <ncurses.h>

#include <string>

//Input: none
//Output: none
//Purpose: declares timer display functions
//Relation: included in any module needing to display timer
std::string countdownTimerText(int remainingSeconds);
std::string countdownTimerText(double remainingSeconds);
void drawCountdownTimer(WINDOW* win,
                        int y,
                        int x,
                        int remainingSeconds,
                        bool hasColor);
void drawCountdownTimer(WINDOW* win,
                        int y,
                        int x,
                        double remainingSeconds,
                        bool hasColor);
void displayCountdownTimer(int seconds, bool hasColor);
