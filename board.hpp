#pragma once

#include <ncurses.h>
#include <string>
#include <vector>

#include "player.hpp"

static const int TILE_COUNT = 89;

//Input: none
//Output: enumerated values representing tile types
//Purpose: defines all possible kinds of tiles on the board
//Relation: used by Tile struct and Board methods to determine logic and rendering
enum TileKind {
    TILE_EMPTY,
    TILE_BLACK,
    TILE_START,
    TILE_SPLIT_START,
    TILE_COLLEGE,
    TILE_CAREER,
    TILE_GRADUATION,
    TILE_MARRIAGE,
    TILE_SPLIT_FAMILY,
    TILE_FAMILY,
    TILE_NIGHT_SCHOOL,
    TILE_SPLIT_RISK,
    TILE_SAFE,
    TILE_RISKY,
    TILE_SPIN_AGAIN,
    TILE_CAREER_2,
    TILE_PAYDAY,
    TILE_BABY,
    TILE_RETIREMENT,
    TILE_HOUSE
};

//Input: none
//Output: holds tile attributes (id, position, label, kind, connections, value, stop flag)
//Purpose: represents a single tile on the board
//Relation: managed by Board, accessed in rendering and movement logic
struct Tile {
    int id;
    int y;
    int x;
    int mode1860Y;
    int mode1860X;
    std::string label;
    TileKind kind;
    int next;
    int altNext;
    int value;
    bool stop;
};

//Input: none
//Output: holds region name and tile index range
//Purpose: groups tiles into named regions for gameplay and UI
//Relation: used by Board::regionNameForTile and tutorialLegend
struct BoardRegion {
    std::string name;
    int startTileIndex;
    int endTileIndex;
};

struct BoardRect {
    int row;
    int col;
    int rows;
    int cols;
};

//Input: none
//Output: enumerated values (FollowCamera, ClassicFull)
//Purpose: defines how the board is displayed to the player
//Relation: used by Board::render to determine view mode
enum class BoardViewMode {
    FollowCamera,
    ClassicFull,
    Mode1860
};

//Input: BoardViewMode or string name
//Output: string name or BoardViewMode
//Purpose: convert between enum and string representation
//Relation: used in UI to display or select view modes
std::string boardViewModeName(BoardViewMode mode);
BoardViewMode boardViewModeFromName(const std::string& name);

//Input: none (constructed object)
//Output: manages tiles and regions
//Purpose: represents the entire game board, including initialization, region logic, and rendering
//Relation: core of the board system, connects Tile, BoardRegion, and rendering functions
class Board {
public:

    //Input: none
    //Output: none
    //Purpose: initializes tiles and regions
    //Relation: calls initTiles and initRegions for setup
    Board();

    //Input: integer id
    //Output: const Tile reference
    //Purpose: retrieves tile by id
    //Relation: used by tutorialLegend, render, and gameplay logic
    const Tile& tileAt(int id) const;
    int tileCount() const;
    bool isMode1860TileId(int id) const;
    int mode1860StartTileId() const;
    int mode1860RetirementTileId() const;
    int mode1860Rows() const;
    int mode1860Cols() const;
    int mode1860TileIdAt(int row, int col) const;
    int mode1860LifeZone(int row, int col) const;
    BoardRect mode1860CameraViewport(int centerTileId, int visibleRows, int visibleCols) const;
    std::vector<int> reachable1860Tiles(int startTileId, int steps) const;
    bool isValid1860Move(int fromTileId, int toTileId, int steps) const;
    //Input: Tile reference
    //Output: bool
    //Purpose: checks if tile is a stop space
    //Relation: used in movement and rendering logic
    bool isStopSpace(const Tile& tile) const;
    //Input: tileIndex
    //Output: string region name
    //Purpose: finds which region a tile belongs to
    //Relation: depends on initRegions, used for UI and tutorial
    std::string regionNameForTile(int tileIndex) const;
    //Input: none
    //Output: vector of strings
    //Purpose: builds tutorial legend with symbols and names
    //Relation: used to display board legend to players
    std::vector<std::string> tutorialLegend() const;
    //Input: boardWin (WINDOW*), players vector, focusPlayerIndex, highlightedTile, hasColor, viewMode
    //Output: none
    //Purpose: draws the entire board, including tiles, regions, landmarks, and player tokens
    //Relation: main visualization function, calls drawClassicBoardGrid, drawClassicRegions, drawClassicLandmarks, drawClassicTile, drawClassicTokens, and drawTileBox
    void render(WINDOW* boardWin,
                const std::vector<Player>& players,
                int focusPlayerIndex,
                int highlightedTile,
                bool hasColor,
                BoardViewMode viewMode = BoardViewMode::FollowCamera) const;
    void render1860Selection(WINDOW* boardWin,
                             const std::vector<Player>& players,
                             int focusPlayerIndex,
                             int cursorTile,
                             const std::vector<int>& reachableTiles,
                             int steps,
                             bool hasColor) const;

private:
    std::vector<Tile> tiles;
    std::vector<Tile> mode1860Tiles;
    std::vector<BoardRegion> regions;

    //Input: none
    //Output: none
    //Purpose: initializes all tiles with positions, labels, kinds, and connections
    //Relation: core setup for board structure, called by constructor
    void initTiles();
    void init1860FreeMovementBoard();
    //Input: none
    //Output: none
    //Purpose: initializes board regions with names and tile ranges
    //Relation: complements initTiles, used by regionNameForTile
    void initRegions();
};
