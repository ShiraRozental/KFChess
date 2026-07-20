#include "doctest/doctest.h"
#include "logic/engine/MovesLog.h"

namespace {
    MoveAppliedEvent knightMove(PieceColor color, long long timestampMs) {
        return MoveAppliedEvent{color, PieceType::Knight,
                                Position{7, 6}, Position{5, 5}, false, timestampMs};
    }
}

TEST_CASE("a move is logged only for its own color") {
    MovesLog log(8);
    log.onMoveApplied(knightMove(PieceColor::White, 100));

    CHECK(log.lastEntriesOf(PieceColor::White, 10).size() == 1);
    CHECK(log.lastEntriesOf(PieceColor::Black, 10).empty());
}

TEST_CASE("a logged entry keeps the event timestamp and notation text") {
    MovesLog log(8);
    log.onMoveApplied(knightMove(PieceColor::White, 1500));

    std::vector<MoveLogEntry> entries = log.lastEntriesOf(PieceColor::White, 10);
    REQUIRE(entries.size() == 1);
    CHECK(entries[0].timestampMs == 1500);
    CHECK(entries[0].text == "Nf3");
}

TEST_CASE("entries keep their arrival order") {
    MovesLog log(8);
    log.onMoveApplied(MoveAppliedEvent{PieceColor::White, PieceType::Pawn,
                                       Position{6, 4}, Position{4, 4}, false, 100});
    log.onMoveApplied(knightMove(PieceColor::White, 200));

    std::vector<MoveLogEntry> entries = log.lastEntriesOf(PieceColor::White, 10);
    REQUIRE(entries.size() == 2);
    CHECK(entries[0].text == "e4");
    CHECK(entries[1].text == "Nf3");
}

TEST_CASE("a jump is logged as Jump") {
    MovesLog log(8);
    log.onMoveApplied(MoveAppliedEvent{PieceColor::Black, PieceType::King,
                                       Position{0, 4}, Position{0, 4}, true, 300});

    std::vector<MoveLogEntry> entries = log.lastEntriesOf(PieceColor::Black, 10);
    REQUIRE(entries.size() == 1);
    CHECK(entries[0].text == "Jump");
}

TEST_CASE("an untouched log has no entries") {
    MovesLog log(8);
    CHECK(log.lastEntriesOf(PieceColor::White, 10).empty());
    CHECK(log.lastEntriesOf(PieceColor::Black, 10).empty());
}

TEST_CASE("lastEntriesOf returns only the newest entries when over the limit") {
    MovesLog log(8);
    for (long long i = 0; i < 5; ++i) {
        log.onMoveApplied(knightMove(PieceColor::White, i * 100));
    }

    std::vector<MoveLogEntry> entries = log.lastEntriesOf(PieceColor::White, 2);
    REQUIRE(entries.size() == 2);
    CHECK(entries[0].timestampMs == 300);
    CHECK(entries[1].timestampMs == 400);
}
