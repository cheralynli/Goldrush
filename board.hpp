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

//Input: none
//Output: rectangle fields for row, column, row count, and column count
//Purpose: describes a visible window over the larger 1860 board
//Relation: returned by Board::mode1860CameraViewport and used by 1860 rendering
struct BoardRect {
    int row;
    int col;
    int rows;
    int cols;
};

//Input: none
//Output: enumerated values (FollowCamera, ClassicFull, Mode1860)
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
    //Input: none
    //Output: total number of classic and 1860 tile ids
    //Purpose: exposes the valid tile-id range for modes that use expanded boards
    //Relation: used by save/load, traps, and validation code
    int tileCount() const;
    //Input: integer tile id
    //Output: bool indicating whether the id belongs to the 1860 board
    //Purpose: separates 1860 grid ids from classic route ids
    //Relation: used by movement, rendering, traps, and save/load migration
    bool isMode1860TileId(int id) const;
    //Input: integer tile id
    //Output: bool indicating whether the 1860 tile may be occupied
    //Purpose: rejects invalid or blank 1860 spaces before movement/traps use them
    //Relation: used by Game 1860 movement and save/load repair
    bool isMode1860WalkableTile(int id) const;
    //Input: none
    //Output: tile id for 1860 start
    //Purpose: centralizes the 1860 start location
    //Relation: used by setup, movement repair, and save/load migration
    int mode1860StartTileId() const;
    //Input: none
    //Output: tile id for 1860 retirement
    //Purpose: centralizes the 1860 retirement location
    //Relation: used by movement, retirement, rendering, and save/load migration
    int mode1860RetirementTileId() const;
    //Input: none
    //Output: number of rows on the 1860 board
    //Purpose: avoids hardcoding 1860 dimensions outside Board
    //Relation: used by Game movement, rendering, and save/load
    int mode1860Rows() const;
    //Input: none
    //Output: number of columns on the 1860 board
    //Purpose: avoids hardcoding 1860 dimensions outside Board
    //Relation: used by Game movement, rendering, and save/load
    int mode1860Cols() const;
    //Input: row and column
    //Output: 1860 tile id, or -1 when coordinates are outside the board
    //Purpose: converts grid coordinates into the actual 1860 tile id
    //Relation: used by rendering and manual movement
    int mode1860TileIdAt(int row, int col) const;
    //Input: row and column
    //Output: life-stage zone from 0 to 5
    //Purpose: maps 1860 board distance into broad life progression zones
    //Relation: used by tile generation, region names, and movement scoring
    int mode1860LifeZone(int row, int col) const;
    //Input: center tile id and requested visible dimensions
    //Output: camera viewport rectangle
    //Purpose: keeps the current 1860 player or cursor visible
    //Relation: used by 1860 camera-follow rendering
    BoardRect mode1860CameraViewport(int centerTileId, int visibleRows, int visibleCols) const;
    //Input: start tile id and movement step count
    //Output: 1860 tile ids at exact Manhattan distance
    //Purpose: supports legacy/debug 1860 distance checks
    //Relation: kept for debug and compatibility with prior 1860 helpers
    std::vector<int> reachable1860Tiles(int startTileId, int steps) const;
    //Input: from tile id, destination tile id, and movement step count
    //Output: bool indicating whether the move has exact Manhattan distance
    //Purpose: supports legacy/debug 1860 distance validation
    //Relation: kept for debug and compatibility with prior 1860 helpers
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
    //Input: board window, players, focus player, cursor tile, reachable tiles, remaining movement, color flag
    //Output: none
    //Purpose: renders 1860 manual movement state with adjacent highlights
    //Relation: used by Game::moveHumanManually1860
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
    //Input: none
    //Output: none
    //Purpose: initializes the 1860 board tile grid and special spaces
    //Relation: called by initTiles for Mode1860 support
    void init1860FreeMovementBoard();
    //Input: none
    //Output: none
    //Purpose: initializes board regions with names and tile ranges
    //Relation: complements initTiles, used by regionNameForTile
    void initRegions();
};
