#pragma once
#include <string>
#include "logic/model/PieceColor.h"

struct PlayerNames {
    std::string white;
    std::string black;

    const std::string& of(PieceColor color) const {
        return color == PieceColor::White ? white : black;
    }
};
