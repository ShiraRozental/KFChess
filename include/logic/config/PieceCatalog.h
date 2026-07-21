#pragma once
#include "logic/config/PieceDefinition.h"
#include <map>
#include <optional>

class PieceCatalog {
public:
    static const PieceCatalog& standard();

    void registerPiece(PieceDefinition definition);
    const PieceDefinition& definitionFor(PieceType type) const;
    std::optional<PieceType> typeForSymbol(char symbol) const;

private:
    std::map<PieceType, PieceDefinition> definitionsByType_;
    std::map<char, PieceType> typesBySymbol_;
};
