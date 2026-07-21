#include "doctest/doctest.h"
#include "shared/protocol/GameStateCodec.h"

namespace {
    constexpr int kBoardRows = 8;
    constexpr int kBoardCols = 8;

    Board boardWith(std::vector<Piece> pieces) {
        Board board(kBoardRows, kBoardCols);
        for (const Piece& piece : pieces) {
            board.addPiece(piece.cell().row, piece.cell().col, piece);
        }
        return board;
    }

    Piece pieceAt(PieceId id, PieceColor color, PieceType kind, Position cell,
                  Piece::State state = Piece::State::Idle) {
        Piece piece(id, color, kind, cell);
        piece.setState(state);
        return piece;
    }

    GameStateMessage messageOf(Board board) {
        return GameStateMessage{GameSnapshot(std::move(board), false, std::nullopt), {}, {}, 0, 0};
    }

    std::optional<GameStateMessage> roundTrip(const GameStateMessage& message) {
        return parseGameState(encodeGameState(message));
    }

    const Piece* pieceOn(const GameStateMessage& message, Position cell) {
        return message.snapshot.board().pieceAt(cell.row, cell.col);
    }

    std::string documentWithPiece(const std::string& pieceJson) {
        return R"({"board":{"rows":8,"cols":8,"pieces":[)" + pieceJson +
               R"(]},"gameOver":false,"cooldowns":[],"inFlight":[],)"
               R"("moves":{"W":[],"B":[]},"score":{"W":0,"B":0}})";
    }

    const std::string kWhiteKing = R"({"id":1,"color":"W","type":"K","row":7,"col":4,"state":"Idle"})";
}

TEST_CASE("a full message survives a round trip") {
    Position e1{7, 4};
    Position d8{0, 3};
    GameStateMessage original{
        GameSnapshot(boardWith({pieceAt(1, PieceColor::White, PieceType::King, e1),
                                pieceAt(2, PieceColor::Black, PieceType::Queen, d8,
                                        Piece::State::LongRest)}),
                     false, std::nullopt,
                     {{2, 0.25}}, {{1, BoardPoint{6.5, 4.0}}}),
        {{1000, "e4"}, {2000, "Nf3"}},
        {{1500, "Jump"}},
        3, 9};

    std::optional<GameStateMessage> parsed = roundTrip(original);
    REQUIRE(parsed.has_value());

    CHECK(parsed->snapshot.board().rowCount() == kBoardRows);
    CHECK(parsed->snapshot.board().colCount() == kBoardCols);
    REQUIRE(pieceOn(*parsed, e1) != nullptr);
    CHECK(pieceOn(*parsed, e1)->id() == 1);
    CHECK(pieceOn(*parsed, e1)->color() == PieceColor::White);
    CHECK(pieceOn(*parsed, e1)->kind() == PieceType::King);
    REQUIRE(pieceOn(*parsed, d8) != nullptr);
    CHECK(pieceOn(*parsed, d8)->state() == Piece::State::LongRest);
    CHECK(parsed->snapshot.cooldownProgressOf(2) == std::optional<double>{0.25});
    REQUIRE(parsed->snapshot.inFlightPositionOf(1).has_value());
    CHECK(parsed->snapshot.inFlightPositionOf(1)->row == doctest::Approx(6.5));
    CHECK(parsed->snapshot.inFlightPositionOf(1)->col == doctest::Approx(4.0));
    CHECK(parsed->whiteScore == 3);
    CHECK(parsed->blackScore == 9);
}

TEST_CASE("an empty board and empty logs survive a round trip") {
    GameStateMessage original{GameSnapshot(Board(0, 0), false, std::nullopt), {}, {}, 0, 0};

    std::optional<GameStateMessage> parsed = roundTrip(original);
    REQUIRE(parsed.has_value());
    CHECK(parsed->snapshot.board().rowCount() == 0);
    CHECK(parsed->snapshot.board().colCount() == 0);
    CHECK(parsed->whiteMoves.empty());
    CHECK(parsed->blackMoves.empty());
    CHECK(parsed->whiteScore == 0);
    CHECK(parsed->blackScore == 0);
}

TEST_CASE("every piece state survives a round trip") {
    Position cell{4, 4};
    for (Piece::State state : {Piece::State::Idle, Piece::State::Moving,
                               Piece::State::Jumping, Piece::State::ShortRest,
                               Piece::State::LongRest, Piece::State::Captured}) {
        GameStateMessage original =
            messageOf(boardWith({pieceAt(1, PieceColor::White, PieceType::Pawn, cell, state)}));

        std::optional<GameStateMessage> parsed = roundTrip(original);
        REQUIRE(parsed.has_value());
        REQUIRE(pieceOn(*parsed, cell) != nullptr);
        CHECK(pieceOn(*parsed, cell)->state() == state);
    }
}

TEST_CASE("every piece type survives a round trip") {
    Position cell{4, 4};
    for (PieceType kind : {PieceType::King, PieceType::Queen, PieceType::Rook,
                           PieceType::Bishop, PieceType::Knight, PieceType::Pawn}) {
        GameStateMessage original =
            messageOf(boardWith({pieceAt(1, PieceColor::White, kind, cell)}));

        std::optional<GameStateMessage> parsed = roundTrip(original);
        REQUIRE(parsed.has_value());
        REQUIRE(pieceOn(*parsed, cell) != nullptr);
        CHECK(pieceOn(*parsed, cell)->kind() == kind);
    }
}

TEST_CASE("both piece colors survive a round trip") {
    Position cell{4, 4};
    for (PieceColor color : {PieceColor::White, PieceColor::Black}) {
        GameStateMessage original =
            messageOf(boardWith({pieceAt(1, color, PieceType::Rook, cell)}));

        std::optional<GameStateMessage> parsed = roundTrip(original);
        REQUIRE(parsed.has_value());
        REQUIRE(pieceOn(*parsed, cell) != nullptr);
        CHECK(pieceOn(*parsed, cell)->color() == color);
    }
}

TEST_CASE("a game with no winner round trips as no winner") {
    GameStateMessage original{GameSnapshot(Board(kBoardRows, kBoardCols), false, std::nullopt),
                              {}, {}, 0, 0};

    std::optional<GameStateMessage> parsed = roundTrip(original);
    REQUIRE(parsed.has_value());
    CHECK_FALSE(parsed->snapshot.isGameOver());
    CHECK_FALSE(parsed->snapshot.winner().has_value());
}

TEST_CASE("a finished game round trips with its winner") {
    GameStateMessage original{
        GameSnapshot(Board(kBoardRows, kBoardCols), true, PieceColor::Black), {}, {}, 0, 0};

    std::optional<GameStateMessage> parsed = roundTrip(original);
    REQUIRE(parsed.has_value());
    CHECK(parsed->snapshot.isGameOver());
    CHECK(parsed->snapshot.winner() == std::optional<PieceColor>{PieceColor::Black});
}

TEST_CASE("a finished game with no winner is legal and round trips") {
    GameStateMessage original{
        GameSnapshot(Board(kBoardRows, kBoardCols), true, std::nullopt), {}, {}, 0, 0};

    std::optional<GameStateMessage> parsed = roundTrip(original);
    REQUIRE(parsed.has_value());
    CHECK(parsed->snapshot.isGameOver());
    CHECK_FALSE(parsed->snapshot.winner().has_value());
}

TEST_CASE("cooldown progress keeps its precision across a round trip") {
    GameStateMessage original{
        GameSnapshot(boardWith({pieceAt(7, PieceColor::White, PieceType::Rook, Position{4, 4},
                                        Piece::State::LongRest)}),
                     false, std::nullopt, {{7, 0.123456789}}),
        {}, {}, 0, 0};

    std::optional<GameStateMessage> parsed = roundTrip(original);
    REQUIRE(parsed.has_value());
    REQUIRE(parsed->snapshot.cooldownProgressOf(7).has_value());
    CHECK(*parsed->snapshot.cooldownProgressOf(7) == 0.123456789);
}

TEST_CASE("a cooldown for a piece that is not on the board is dropped") {
    GameStateMessage original{
        GameSnapshot(Board(kBoardRows, kBoardCols), false, std::nullopt, {{7, 0.5}}),
        {}, {}, 0, 0};

    std::optional<GameStateMessage> parsed = roundTrip(original);
    REQUIRE(parsed.has_value());
    CHECK_FALSE(parsed->snapshot.cooldownProgressOf(7).has_value());
}

TEST_CASE("move log order and per-color separation survive a round trip") {
    GameStateMessage original{GameSnapshot(Board(0, 0), false, std::nullopt),
                              {{100, "e4"}, {200, "d4"}, {300, "Nf3"}},
                              {},
                              0, 0};

    std::optional<GameStateMessage> parsed = roundTrip(original);
    REQUIRE(parsed.has_value());
    REQUIRE(parsed->whiteMoves.size() == 3);
    CHECK(parsed->blackMoves.empty());
    CHECK(parsed->whiteMoves[0].timestampMs == 100);
    CHECK(parsed->whiteMoves[0].text == "e4");
    CHECK(parsed->whiteMoves[2].timestampMs == 300);
    CHECK(parsed->whiteMoves[2].text == "Nf3");
}

TEST_CASE("a valid document parses") {
    CHECK(parseGameState(documentWithPiece(kWhiteKing)).has_value());
}

TEST_CASE("malformed json is rejected") {
    CHECK_FALSE(parseGameState("{ not json").has_value());
}

TEST_CASE("an empty string is rejected") {
    CHECK_FALSE(parseGameState("").has_value());
}

TEST_CASE("a root that is not an object is rejected") {
    CHECK_FALSE(parseGameState("[]").has_value());
    CHECK_FALSE(parseGameState("5").has_value());
}

TEST_CASE("a missing top level field is rejected") {
    CHECK_FALSE(parseGameState(
        R"({"gameOver":false,"cooldowns":[],"inFlight":[],)"
        R"("moves":{"W":[],"B":[]},"score":{"W":0,"B":0}})").has_value());
    CHECK_FALSE(parseGameState(
        R"({"board":{"rows":8,"cols":8,"pieces":[]},"gameOver":false,)"
        R"("cooldowns":[],"inFlight":[],"moves":{"W":[],"B":[]}})").has_value());
    CHECK_FALSE(parseGameState(
        R"({"board":{"rows":8,"cols":8,"pieces":[]},"gameOver":false,)"
        R"("cooldowns":[],"inFlight":[],"score":{"W":0,"B":0}})").has_value());
}

TEST_CASE("a field of the wrong json type is rejected") {
    CHECK_FALSE(parseGameState(
        R"({"board":{"rows":"8","cols":8,"pieces":[]},"gameOver":false,)"
        R"("cooldowns":[],"inFlight":[],"moves":{"W":[],"B":[]},"score":{"W":0,"B":0}})")
        .has_value());
}

TEST_CASE("the board text format's lowercase color is rejected") {
    CHECK_FALSE(parseGameState(documentWithPiece(
        R"({"id":1,"color":"w","type":"K","row":7,"col":4,"state":"Idle"})")).has_value());
}

TEST_CASE("an unknown piece type letter is rejected") {
    CHECK_FALSE(parseGameState(documentWithPiece(
        R"({"id":1,"color":"W","type":"Z","row":7,"col":4,"state":"Idle"})")).has_value());
}

TEST_CASE("a lowercase piece type letter is rejected") {
    CHECK_FALSE(parseGameState(documentWithPiece(
        R"({"id":1,"color":"W","type":"q","row":7,"col":4,"state":"Idle"})")).has_value());
}

TEST_CASE("an unknown piece state name is rejected") {
    CHECK_FALSE(parseGameState(documentWithPiece(
        R"({"id":1,"color":"W","type":"K","row":7,"col":4,"state":"Dancing"})")).has_value());
}

TEST_CASE("a piece outside the declared board is rejected") {
    CHECK_FALSE(parseGameState(documentWithPiece(
        R"({"id":1,"color":"W","type":"K","row":99,"col":4,"state":"Idle"})")).has_value());
    CHECK_FALSE(parseGameState(documentWithPiece(
        R"({"id":1,"color":"W","type":"K","row":-1,"col":4,"state":"Idle"})")).has_value());
}

TEST_CASE("two pieces on the same cell are rejected") {
    CHECK_FALSE(parseGameState(documentWithPiece(
        kWhiteKing + R"(,{"id":2,"color":"B","type":"Q","row":7,"col":4,"state":"Idle"})"))
        .has_value());
}
