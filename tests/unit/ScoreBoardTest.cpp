#include "doctest/doctest.h"
#include "engine/GameEngine.h"
#include "engine/MovesLog.h"
#include "engine/ScoreBoard.h"
#include "io/BoardParser.h"
#include <sstream>

namespace {
    PieceCapturedEvent captureOf(PieceType kind, PieceColor capturedBy, long long timestampMs) {
        PieceColor capturedColor =
            capturedBy == PieceColor::White ? PieceColor::Black : PieceColor::White;
        return PieceCapturedEvent{capturedColor, kind, capturedBy, timestampMs};
    }

    GameEngine makeGame(const std::string& boardText) {
        std::istringstream in(boardText);
        ParsedInput parsed;
        std::string error;
        BoardParser::parse(in, parsed, error);
        return GameEngine(std::move(parsed.board));
    }
}

TEST_CASE("both players start with a zero score") {
    ScoreBoard score;
    CHECK(score.scoreOf(PieceColor::White) == 0);
    CHECK(score.scoreOf(PieceColor::Black) == 0);
}

TEST_CASE("a capture credits only the capturing player") {
    ScoreBoard score;
    score.onPieceCaptured(captureOf(PieceType::Knight, PieceColor::White, 100));

    CHECK(score.scoreOf(PieceColor::White) == 3);
    CHECK(score.scoreOf(PieceColor::Black) == 0);
}

TEST_CASE("captures accumulate") {
    ScoreBoard score;
    score.onPieceCaptured(captureOf(PieceType::Knight, PieceColor::White, 100));
    score.onPieceCaptured(captureOf(PieceType::Queen, PieceColor::White, 200));

    CHECK(score.scoreOf(PieceColor::White) == 12);
}

TEST_CASE("each player accumulates an independent score") {
    ScoreBoard score;
    score.onPieceCaptured(captureOf(PieceType::Rook, PieceColor::White, 100));
    score.onPieceCaptured(captureOf(PieceType::Pawn, PieceColor::Black, 200));

    CHECK(score.scoreOf(PieceColor::White) == 5);
    CHECK(score.scoreOf(PieceColor::Black) == 1);
}

TEST_CASE("capturing a king adds no points") {
    ScoreBoard score;
    score.onPieceCaptured(captureOf(PieceType::King, PieceColor::Black, 100));
    CHECK(score.scoreOf(PieceColor::Black) == 0);
}

TEST_CASE("an engine capture reaches registered score and log observers") {
    GameEngine game = makeGame("Board:\nwR . bN\n");
    ScoreBoard score;
    MovesLog log(1);
    game.addListener(score);
    game.addListener(log);

    game.requestMove(Position{0, 0}, Position{0, 2});
    game.wait(2000);

    CHECK(score.scoreOf(PieceColor::White) == 3);
    std::vector<MoveLogEntry> entries = log.lastEntriesOf(PieceColor::White, 10);
    REQUIRE(entries.size() == 1);
    CHECK(entries[0].text == "Rc1");
}
