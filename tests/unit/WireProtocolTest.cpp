#include "doctest/doctest.h"
#include "shared/protocol/WireProtocol.h"

namespace {
    constexpr int kBoardRows = 8;

    // Board cells used across the cases (row, col), with their algebraic names:
    // e2 = {6,4}, e5 = {3,4}, g1 = {7,6}, a1 = {7,0}, h8 = {0,7}.
    const Position e2{6, 4};
    const Position e5{3, 4};
    const Position g1{7, 6};
    const Position a1{7, 0};
    const Position h8{0, 7};
}

TEST_CASE("encode move: white queen e2 to e5 -> WQe2e5") {
    CHECK(encodeMove(PieceColor::White, PieceType::Queen, e2, e5, kBoardRows) == "WQe2e5");
}

TEST_CASE("encode move: black bishop -> BB, color and piece not mixed up") {
    CHECK(encodeMove(PieceColor::Black, PieceType::Bishop, e2, e5, kBoardRows) == "BBe2e5");
}

TEST_CASE("encode jump: white knight g1 -> WNg1 (4 chars)") {
    CHECK(encodeJump(PieceColor::White, PieceType::Knight, g1, kBoardRows) == "WNg1");
}

TEST_CASE("encode move: board corners a1 and h8") {
    CHECK(encodeMove(PieceColor::White, PieceType::Rook, a1, h8, kBoardRows) == "WRa1h8");
}

TEST_CASE("parse move: WQe2e5 -> white queen, from e2, to e5") {
    std::optional<WireCommand> parsed = parseCommand("WQe2e5", kBoardRows);
    REQUIRE(parsed.has_value());
    CHECK(parsed->type == WireCommandType::Move);
    CHECK(parsed->color == PieceColor::White);
    CHECK(parsed->piece == PieceType::Queen);
    CHECK(parsed->from == e2);
    CHECK(parsed->to == e5);
}

TEST_CASE("parse jump: WNg1 -> white knight, cell g1") {
    std::optional<WireCommand> parsed = parseCommand("WNg1", kBoardRows);
    REQUIRE(parsed.has_value());
    CHECK(parsed->type == WireCommandType::Jump);
    CHECK(parsed->color == PieceColor::White);
    CHECK(parsed->piece == PieceType::Knight);
    CHECK(parsed->from == g1);
}

TEST_CASE("parse: BBe2e5 -> B at pos 0 is color (black), B at pos 1 is bishop") {
    std::optional<WireCommand> parsed = parseCommand("BBe2e5", kBoardRows);
    REQUIRE(parsed.has_value());
    CHECK(parsed->color == PieceColor::Black);
    CHECK(parsed->piece == PieceType::Bishop);
}

TEST_CASE("parse rejects wrong length") {
    CHECK_FALSE(parseCommand("", kBoardRows).has_value());
    CHECK_FALSE(parseCommand("WQe", kBoardRows).has_value());
    CHECK_FALSE(parseCommand("WQe2e", kBoardRows).has_value());
    CHECK_FALSE(parseCommand("WQe2e5x", kBoardRows).has_value());
}

TEST_CASE("parse rejects a bad color letter") {
    CHECK_FALSE(parseCommand("wQe2e5", kBoardRows).has_value());
    CHECK_FALSE(parseCommand("XQe2e5", kBoardRows).has_value());
}

TEST_CASE("parse rejects a bad piece letter") {
    CHECK_FALSE(parseCommand("WZe2e5", kBoardRows).has_value());
}

TEST_CASE("parse rejects a cell off the board") {
    CHECK_FALSE(parseCommand("WQi2e5", kBoardRows).has_value());  // file too far
    CHECK_FALSE(parseCommand("WQe0e5", kBoardRows).has_value());  // rank 0
    CHECK_FALSE(parseCommand("WQe9e5", kBoardRows).has_value());  // rank 9
}

TEST_CASE("round trip: encode a move then parse it gives the same command") {
    std::string wire = encodeMove(PieceColor::Black, PieceType::Pawn, e5, e2, kBoardRows);
    std::optional<WireCommand> parsed = parseCommand(wire, kBoardRows);
    REQUIRE(parsed.has_value());
    CHECK(*parsed == WireCommand{WireCommandType::Move, PieceColor::Black,
                                 PieceType::Pawn, e5, e2});
}

TEST_CASE("round trip: encode a jump then parse it gives the same command") {
    std::string wire = encodeJump(PieceColor::White, PieceType::King, e2, kBoardRows);
    std::optional<WireCommand> parsed = parseCommand(wire, kBoardRows);
    REQUIRE(parsed.has_value());
    CHECK(*parsed == WireCommand{WireCommandType::Jump, PieceColor::White,
                                 PieceType::King, e2, e2});
}
