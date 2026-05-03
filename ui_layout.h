#pragma once

//Input: none 
//Output: holds overall UILayout attributes
//Purpose: represents the dimensions and bounds of the main game UI
//Relation: accessed in creating the UI
struct UILayout {
    bool compact;
    int headerHeight;
    int boardWidth;
    int boardHeight;
    int sidePanelWidth;
    int sidePanelHeight;
    int messageHeight;
    int totalWidth;
    int totalHeight;
    int originX;
    int originY;
};

//Input: int termHeight, int termWidth (terminal height/width)
//Output: UILayout calculateUILayout
//Purpose: determine UI size based on size of terminal
//Relation: used in createWindows and chooseBoardViewMode in game.cpp
UILayout calculateUILayout(int termHeight = -1, int termWidth = -1);

//Input: none
//Output: int minimumGameWidth/minimumGameHeight
//Purpose: minimum terminal width/height to run game
//Relation: used in ensureMinSize
int minimumGameWidth();
int minimumGameHeight();
