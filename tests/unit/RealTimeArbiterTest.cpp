#include "doctest/doctest.h"
#include "engine/RealTimeArbiter.h"

namespace {
    Piece makePiece(Position cell) {
        return Piece(1, PieceColor::White, PieceType::Rook, cell);
    }
}

TEST_CASE("hasActiveMotion is false with nothing in flight") {
    RealTimeArbiter arbiter;
    CHECK_FALSE(arbiter.hasActiveMotion());
}

TEST_CASE("starting a move makes hasActiveMotion true") {
    RealTimeArbiter arbiter;
    Piece piece = makePiece(Position{0, 0});
    arbiter.startMotion(piece, Position{0, 0}, Position{0, 2});
    CHECK(arbiter.hasActiveMotion());
}

TEST_CASE("starting a jump (source == destination) does not count as an active motion") {
    RealTimeArbiter arbiter;
    Piece piece = makePiece(Position{1, 1});
    arbiter.startMotion(piece, Position{1, 1}, Position{1, 1});
    CHECK_FALSE(arbiter.hasActiveMotion());
}

TEST_CASE("startMotion sets the piece state to Moving") {
    RealTimeArbiter arbiter;
    Piece piece = makePiece(Position{0, 0});
    CHECK(piece.state() == Piece::State::Idle);
    arbiter.startMotion(piece, Position{0, 0}, Position{0, 1});
    CHECK(piece.state() == Piece::State::Moving);
}

TEST_CASE("a move that has not reached its arrival time yet produces no events") {
    RealTimeArbiter arbiter;
    Piece piece = makePiece(Position{0, 0});
    arbiter.startMotion(piece, Position{0, 0}, Position{0, 1});
    ArrivalEvents events = arbiter.advanceTime(999);
    CHECK(events.empty());
    CHECK(arbiter.hasActiveMotion());
}

TEST_CASE("a one-cell move arrives after exactly 1000ms") {
    RealTimeArbiter arbiter;
    Piece piece = makePiece(Position{0, 0});
    arbiter.startMotion(piece, Position{0, 0}, Position{0, 1});
    ArrivalEvents events = arbiter.advanceTime(1000);
    REQUIRE(events.size() == 1);
    CHECK(events[0].from == Position{0, 0});
    CHECK(events[0].to == Position{0, 1});
    CHECK_FALSE(events[0].intercepted);
}

TEST_CASE("a two-cell move takes 2000ms") {
    RealTimeArbiter arbiter;
    Piece piece = makePiece(Position{0, 0});
    arbiter.startMotion(piece, Position{0, 0}, Position{0, 2});
    CHECK(arbiter.advanceTime(1000).empty());
    ArrivalEvents events = arbiter.advanceTime(1000);
    CHECK(events.size() == 1);
}

TEST_CASE("a diagonal move uses Chebyshev distance, not Euclidean distance") {
    RealTimeArbiter arbiter;
    Piece piece = makePiece(Position{0, 0});
    arbiter.startMotion(piece, Position{0, 0}, Position{3, 3});
    CHECK(arbiter.advanceTime(2999).empty());
    ArrivalEvents events = arbiter.advanceTime(1);
    CHECK(events.size() == 1);
}

TEST_CASE("hasActiveMotion becomes false and the piece returns to Idle after arrival") {
    RealTimeArbiter arbiter;
    Piece piece = makePiece(Position{0, 0});
    arbiter.startMotion(piece, Position{0, 0}, Position{0, 1});
    arbiter.advanceTime(1000);
    CHECK_FALSE(arbiter.hasActiveMotion());
    CHECK(piece.state() == Piece::State::Idle);
}

TEST_CASE("a jump landing with nothing arriving produces no events and returns the piece to Idle") {
    RealTimeArbiter arbiter;
    Piece piece = makePiece(Position{1, 1});
    arbiter.startMotion(piece, Position{1, 1}, Position{1, 1});
    ArrivalEvents events = arbiter.advanceTime(1000);
    CHECK(events.empty());
    CHECK(piece.state() == Piece::State::Idle);
}

TEST_CASE("a move arriving at a cell with a still-active jump is intercepted") {
    RealTimeArbiter arbiter;
    Piece jumper = makePiece(Position{1, 0});
    Piece attacker = makePiece(Position{1, 1});
    arbiter.startMotion(jumper, Position{1, 0}, Position{1, 0});   // airborne until t=1000
    arbiter.startMotion(attacker, Position{1, 1}, Position{1, 0}); // arrives at t=1000, same cell

    ArrivalEvents events = arbiter.advanceTime(1000);
    REQUIRE(events.size() == 1);
    CHECK(events[0].intercepted);
    CHECK(attacker.state() == Piece::State::Captured);
}

TEST_CASE("an enemy arriving after a jump has already landed captures normally, not intercepted") {
    RealTimeArbiter arbiter;
    Piece jumper = makePiece(Position{1, 0});
    Piece attacker = makePiece(Position{1, 3});
    arbiter.startMotion(jumper, Position{1, 0}, Position{1, 0}); // airborne until t=1000
    arbiter.advanceTime(1000);                                    // jump lands, no longer protected
    arbiter.startMotion(attacker, Position{1, 3}, Position{1, 0}); // distance 3: arrives at t=4000

    ArrivalEvents events = arbiter.advanceTime(3000);
    REQUIRE(events.size() == 1);
    CHECK_FALSE(events[0].intercepted);
}

TEST_CASE("two partial waits that sum to the full duration deliver the same arrival as one full wait") {
    RealTimeArbiter arbiter;
    Piece piece = makePiece(Position{0, 0});
    arbiter.startMotion(piece, Position{0, 0}, Position{0, 1});
    CHECK(arbiter.advanceTime(400).empty());
    ArrivalEvents events = arbiter.advanceTime(600);
    CHECK(events.size() == 1);
}
