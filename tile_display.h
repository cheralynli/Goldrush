#pragma once

#include <string>

#include "board.hpp"

//Input: none
//Output: none
//Purpose: declares tile display functions
//Relation: included in any module needing to display tile label information
std::string getTileFullName(TileKind kind);
std::string getTileFullName(const Tile& tile);
std::string getTileAbbreviation(TileKind kind);
std::string getTileAbbreviation(const Tile& tile);
std::string getTileBoardSymbol(TileKind kind);
std::string getTileBoardSymbol(const Tile& tile);
std::string getTileDisplayName(TileKind kind);
std::string getTileDisplayName(const Tile& tile);
