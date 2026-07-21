#include "logic/config/PieceCatalog.h"
#include "logic/rules/PieceRules.h"
#include <stdexcept>

namespace {
    constexpr int kPawnCost = 1;
    constexpr int kKnightCost = 3;
    constexpr int kBishopCost = 3;
    constexpr int kRookCost = 5;
    constexpr int kQueenCost = 9;
    constexpr int kKingCost = 0;
    constexpr long long kDefaultCooldownMs = 1000;

    constexpr char kKingSymbol = 'K';
    constexpr char kQueenSymbol = 'Q';
    constexpr char kRookSymbol = 'R';
    constexpr char kBishopSymbol = 'B';
    constexpr char kKnightSymbol = 'N';
    constexpr char kPawnSymbol = 'P';

    template <typename Rule>
    PieceDefinition colorBlindPiece(PieceType type, char symbol, int captureCost,
                                    long long cooldownMs = kDefaultCooldownMs) {
        auto rule = std::make_shared<const Rule>();
        return PieceDefinition{type, symbol, captureCost, cooldownMs, type, rule, rule};
    }

    PieceDefinition pawnPiece() {
        return PieceDefinition{PieceType::Pawn, kPawnSymbol, kPawnCost, kDefaultCooldownMs,
                               PieceType::Queen,
                               std::make_shared<const WhitePawnMovementRule>(),
                               std::make_shared<const BlackPawnMovementRule>()};
    }

    PieceCatalog buildStandardCatalog() {
        PieceCatalog catalog;
        catalog.registerPiece(colorBlindPiece<KingMovementRule>(PieceType::King, kKingSymbol, kKingCost));
        catalog.registerPiece(colorBlindPiece<QueenMovementRule>(PieceType::Queen, kQueenSymbol, kQueenCost));
        catalog.registerPiece(colorBlindPiece<RookMovementRule>(PieceType::Rook, kRookSymbol, kRookCost));
        catalog.registerPiece(colorBlindPiece<BishopMovementRule>(PieceType::Bishop, kBishopSymbol, kBishopCost));
        catalog.registerPiece(colorBlindPiece<KnightMovementRule>(PieceType::Knight, kKnightSymbol, kKnightCost));
        catalog.registerPiece(pawnPiece());
        return catalog;
    }
}

const PieceCatalog& PieceCatalog::standard() {
    static const PieceCatalog catalog = buildStandardCatalog();
    return catalog;
}

void PieceCatalog::registerPiece(PieceDefinition definition) {
    if (definitionsByType_.count(definition.type)) {
        throw std::invalid_argument("Piece type is already registered in this catalog");
    }
    if (typesBySymbol_.count(definition.symbol)) {
        throw std::invalid_argument(std::string("Piece symbol is already taken: ") + definition.symbol);
    }
    typesBySymbol_[definition.symbol] = definition.type;
    definitionsByType_.emplace(definition.type, std::move(definition));
}

const PieceDefinition& PieceCatalog::definitionFor(PieceType type) const {
    auto found = definitionsByType_.find(type);
    if (found == definitionsByType_.end()) {
        throw std::out_of_range("No piece definition registered for this type");
    }
    return found->second;
}

std::optional<PieceType> PieceCatalog::typeForSymbol(char symbol) const {
    auto found = typesBySymbol_.find(symbol);
    if (found == typesBySymbol_.end()) return std::nullopt;
    return found->second;
}
