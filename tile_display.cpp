#include "tile_display.h"

std::string getTileFullName(TileKind kind) {
    switch (kind) {
        case TILE_BLACK: return "Black Tile";
        case TILE_START: return "Start";
        case TILE_SPLIT_START: return "Checkered Choice";
        case TILE_COLLEGE: return "College";
        case TILE_CAREER: return "Career";
        case TILE_GRADUATION: return "Graduation";
        case TILE_MARRIAGE: return "Marriage";
        case TILE_SPLIT_FAMILY: return "Family Split";
        case TILE_FAMILY: return "Family";
        case TILE_NIGHT_SCHOOL: return "Night School";
        case TILE_SPLIT_RISK: return "Risk Split";
        case TILE_SAFE: return "Safe";
        case TILE_RISKY: return "Risk";
        case TILE_SPIN_AGAIN: return "Spin Again";
        case TILE_CAREER_2: return "Work";
        case TILE_PAYDAY: return "Payday";
        case TILE_BABY: return "Family Event";
        case TILE_RETIREMENT: return "Retirement";
        case TILE_HOUSE: return "House";
        case TILE_EMPTY:
        default:
            return "Open Road";
    }
}

std::string getTileFullName(const Tile& tile) {
    if (tile.kind == TILE_BLACK) {
        return "Black Tile";
    }
    return getTileFullName(tile.kind);
}

std::string getTileAbbreviation(TileKind kind) {
    switch (kind) {
        case TILE_BLACK: return "c";
        case TILE_START: return "ST";
        case TILE_SPLIT_START: return "CH";
        case TILE_COLLEGE: return "CO";
        case TILE_CAREER: return "CA";
        case TILE_GRADUATION: return "GR";
        case TILE_MARRIAGE: return "MA";
        case TILE_SPLIT_FAMILY: return "FS";
        case TILE_FAMILY: return "FA";
        case TILE_NIGHT_SCHOOL: return "NS";
        case TILE_SPLIT_RISK: return "RS";
        case TILE_SAFE: return "SA";
        case TILE_RISKY: return "RI";
        case TILE_SPIN_AGAIN: return "SG";
        case TILE_CAREER_2: return "WK";
        case TILE_PAYDAY: return "PD";
        case TILE_BABY: return "FE";
        case TILE_RETIREMENT: return "RT";
        case TILE_HOUSE: return "HS";
        case TILE_EMPTY:
        default:
            return "--";
    }
}

std::string getTileAbbreviation(const Tile& tile) {
    if (tile.kind == TILE_BLACK) {
        return "c";
    }
    if (tile.kind != TILE_EMPTY && tile.kind != TILE_BLACK && !tile.label.empty()) {
        return tile.label;
    }
    return getTileAbbreviation(tile.kind);
}

std::string getTileBoardSymbol(TileKind kind) {
    switch (kind) {
        case TILE_BLACK: return "c";
        case TILE_START: return "ST";
        case TILE_SPLIT_START: return "<>";
        case TILE_COLLEGE: return "CO";
        case TILE_CAREER: return "CA";
        case TILE_GRADUATION: return "GR";
        case TILE_MARRIAGE: return "MR";
        case TILE_SPLIT_FAMILY: return "Y ";
        case TILE_FAMILY: return "FA";
        case TILE_NIGHT_SCHOOL: return "NS";
        case TILE_SPLIT_RISK: return "??";
        case TILE_SAFE: return "S";
        case TILE_RISKY: return "R";
        case TILE_SPIN_AGAIN: return ">>";
        case TILE_CAREER_2: return "WK";
        case TILE_PAYDAY: return "$";
        case TILE_BABY: return "B ";
        case TILE_RETIREMENT: return "RT";
        case TILE_HOUSE: return "HS";
        case TILE_EMPTY:
        default:
            return "  ";
    }
}

std::string getTileBoardSymbol(const Tile& tile) {
    if (tile.kind == TILE_BLACK) {
        return "c";
    }
    return getTileBoardSymbol(tile.kind);
}

std::string getTileDisplayName(TileKind kind) {
    return getTileFullName(kind) + " (" + getTileAbbreviation(kind) + ")";
}

std::string getTileDisplayName(const Tile& tile) {
    return getTileFullName(tile) + " (" + getTileAbbreviation(tile) + ")";
}
