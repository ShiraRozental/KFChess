#include "doctest/doctest.h"
#include "animation/PieceAssetPaths.h"
#include <filesystem>

namespace {
    std::filesystem::path fixtureRoot(const std::string& name) {
        return std::filesystem::path(TEST_FIXTURES_DIR) / "animation_loader" / name;
    }
}

TEST_CASE("pieceAssetCode orders type before color, both uppercase") {
    CHECK(pieceAssetCode(PieceColor::White, PieceType::King) == "KW");
    CHECK(pieceAssetCode(PieceColor::Black, PieceType::King) == "KB");
    CHECK(pieceAssetCode(PieceColor::White, PieceType::Queen) == "QW");
    CHECK(pieceAssetCode(PieceColor::Black, PieceType::Rook) == "RB");
    CHECK(pieceAssetCode(PieceColor::White, PieceType::Bishop) == "BW");
    CHECK(pieceAssetCode(PieceColor::Black, PieceType::Knight) == "NB");
    CHECK(pieceAssetCode(PieceColor::White, PieceType::Pawn) == "PW");
}

TEST_CASE("resolveStateFolders resolves all five states for a complete piece") {
    auto folders = resolveStateFolders(PieceColor::White, PieceType::King, fixtureRoot("complete"));

    CHECK(folders.size() == 5);
    for (const auto& state : {"idle", "move", "jump", "short_rest", "long_rest"}) {
        REQUIRE(folders.count(state) == 1);
        CHECK(std::filesystem::is_directory(folders.at(state)));
    }
}

TEST_CASE("a missing state folder is a clear rejection, not a crash") {
    CHECK_THROWS_AS(resolveStateFolders(PieceColor::White, PieceType::King, fixtureRoot("incomplete")),
                     std::runtime_error);
}

TEST_CASE("a missing piece folder entirely is also a clear rejection") {
    CHECK_THROWS_AS(resolveStateFolders(PieceColor::Black, PieceType::King, fixtureRoot("complete")),
                     std::runtime_error);
}
