#include "doctest/doctest.h"
#include "logic/config/PieceCatalog.h"
#include "logic/io/PieceNotation.h"
#include "logic/rules/PieceRules.h"
#include <array>
#include <stdexcept>

namespace {
    const std::array<PieceType, 6> kAllTypes = {
        PieceType::King, PieceType::Queen, PieceType::Rook,
        PieceType::Bishop, PieceType::Knight, PieceType::Pawn};

    PieceDefinition rookLike(PieceType type, char symbol) {
        auto rule = std::make_shared<const RookMovementRule>();
        return PieceDefinition{type, symbol, 5, 1000, type, rule, rule};
    }
}

TEST_CASE("a registered definition is returned unchanged") {
    PieceCatalog catalog;
    catalog.registerPiece(rookLike(PieceType::Rook, 'R'));

    const PieceDefinition& definition = catalog.definitionFor(PieceType::Rook);
    CHECK(definition.symbol == 'R');
    CHECK(definition.captureCost == 5);
    CHECK(definition.cooldownMs == 1000);
}

TEST_CASE("asking for an unregistered type is a programming error, not a silent default") {
    PieceCatalog catalog;
    CHECK_THROWS_AS(catalog.definitionFor(PieceType::Queen), std::out_of_range);
}

TEST_CASE("a symbol resolves back to its registered type") {
    PieceCatalog catalog;
    catalog.registerPiece(rookLike(PieceType::Rook, 'R'));

    std::optional<PieceType> found = catalog.typeForSymbol('R');
    REQUIRE(found.has_value());
    CHECK(*found == PieceType::Rook);
}

TEST_CASE("an unknown symbol yields nullopt instead of throwing") {
    PieceCatalog catalog;
    catalog.registerPiece(rookLike(PieceType::Rook, 'R'));

    CHECK_FALSE(catalog.typeForSymbol('Z').has_value());
}

TEST_CASE("registering two pieces under one symbol is rejected") {
    PieceCatalog catalog;
    catalog.registerPiece(rookLike(PieceType::Rook, 'R'));

    CHECK_THROWS_AS(catalog.registerPiece(rookLike(PieceType::Queen, 'R')),
                    std::invalid_argument);
}

TEST_CASE("registering one type twice is rejected") {
    PieceCatalog catalog;
    catalog.registerPiece(rookLike(PieceType::Rook, 'R'));

    CHECK_THROWS_AS(catalog.registerPiece(rookLike(PieceType::Rook, 'X')),
                    std::invalid_argument);
}

TEST_CASE("a rejected registration leaves the catalog untouched") {
    PieceCatalog catalog;
    catalog.registerPiece(rookLike(PieceType::Rook, 'R'));
    CHECK_THROWS(catalog.registerPiece(rookLike(PieceType::Queen, 'R')));

    CHECK_FALSE(catalog.typeForSymbol('R') == std::optional<PieceType>{PieceType::Queen});
    CHECK_THROWS_AS(catalog.definitionFor(PieceType::Queen), std::out_of_range);
}

TEST_CASE("the standard catalog registers every piece type") {
    const PieceCatalog& catalog = PieceCatalog::standard();
    for (PieceType type : kAllTypes) {
        CHECK_NOTHROW(catalog.definitionFor(type));
    }
}

TEST_CASE("the standard catalog keeps the established piece symbols") {
    const PieceCatalog& catalog = PieceCatalog::standard();
    CHECK(catalog.definitionFor(PieceType::King).symbol == 'K');
    CHECK(catalog.definitionFor(PieceType::Queen).symbol == 'Q');
    CHECK(catalog.definitionFor(PieceType::Rook).symbol == 'R');
    CHECK(catalog.definitionFor(PieceType::Bishop).symbol == 'B');
    CHECK(catalog.definitionFor(PieceType::Knight).symbol == 'N');
    CHECK(catalog.definitionFor(PieceType::Pawn).symbol == 'P');
}

TEST_CASE("the standard catalog keeps the established capture costs") {
    const PieceCatalog& catalog = PieceCatalog::standard();
    CHECK(catalog.definitionFor(PieceType::Pawn).captureCost == 1);
    CHECK(catalog.definitionFor(PieceType::Knight).captureCost == 3);
    CHECK(catalog.definitionFor(PieceType::Bishop).captureCost == 3);
    CHECK(catalog.definitionFor(PieceType::Rook).captureCost == 5);
    CHECK(catalog.definitionFor(PieceType::Queen).captureCost == 9);
    CHECK(catalog.definitionFor(PieceType::King).captureCost == 0);
}

TEST_CASE("every standard piece has a positive cooldown") {
    const PieceCatalog& catalog = PieceCatalog::standard();
    for (PieceType type : kAllTypes) {
        CHECK(catalog.definitionFor(type).cooldownMs > 0);
    }
}

TEST_CASE("the standard catalog is a single shared instance") {
    CHECK(&PieceCatalog::standard() == &PieceCatalog::standard());
}

TEST_CASE("only the pawn promotes, and it promotes to a queen") {
    const PieceCatalog& catalog = PieceCatalog::standard();
    CHECK(catalog.definitionFor(PieceType::Pawn).promotesTo == PieceType::Queen);
    for (PieceType type : kAllTypes) {
        if (type == PieceType::Pawn) continue;
        CHECK(catalog.definitionFor(type).promotesTo == type);
    }
}

TEST_CASE("the pawn is the only piece whose movement rule depends on its color") {
    const PieceCatalog& catalog = PieceCatalog::standard();

    const PieceDefinition& pawn = catalog.definitionFor(PieceType::Pawn);
    CHECK(&pawn.movementFor(PieceColor::White) != &pawn.movementFor(PieceColor::Black));

    for (PieceType type : kAllTypes) {
        if (type == PieceType::Pawn) continue;
        const PieceDefinition& definition = catalog.definitionFor(type);
        CHECK(&definition.movementFor(PieceColor::White) ==
              &definition.movementFor(PieceColor::Black));
    }
}

TEST_CASE("the notation accessors agree with the standard catalog") {
    const PieceCatalog& catalog = PieceCatalog::standard();
    for (PieceType type : kAllTypes) {
        const PieceDefinition& definition = catalog.definitionFor(type);
        CHECK(pieceTypeToChar(type) == definition.symbol);
        CHECK(pieceTypeFromChar(definition.symbol) == std::optional<PieceType>{type});
    }
}
