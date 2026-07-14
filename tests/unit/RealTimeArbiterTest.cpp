#include "doctest/doctest.h"
#include "realtime/RealTimeArbiter.h"

namespace {
    Piece makePiece(Position cell, PieceId id = 1, PieceColor color = PieceColor::White) {
        return Piece(id, color, PieceType::Rook, cell);
    }
}

TEST_CASE("hasActiveMotion is false with nothing in flight") {
    RealTimeArbiter arbiter;
    CHECK_FALSE(arbiter.hasActiveMotion());
}

TEST_CASE("starting a move makes hasActiveMotion true") {
    RealTimeArbiter arbiter;
    arbiter.startMotion(makePiece(Position{0, 0}), Position{0, 0}, Position{0, 2});
    CHECK(arbiter.hasActiveMotion());
}

TEST_CASE("starting a jump (source == destination) does not count as an active motion") {
    RealTimeArbiter arbiter;
    arbiter.startMotion(makePiece(Position{1, 1}), Position{1, 1}, Position{1, 1});
    CHECK_FALSE(arbiter.hasActiveMotion());
}

TEST_CASE("startMotion takes ownership and sets the in-flight piece's state to Moving") {
    RealTimeArbiter arbiter;
    Piece original = makePiece(Position{0, 0});
    arbiter.startMotion(original, Position{0, 0}, Position{0, 1});

    std::vector<InFlightPiece> fliers = arbiter.inFlightPieces();
    REQUIRE(fliers.size() == 1);
    CHECK(fliers[0].piece.id() == original.id());
    CHECK(fliers[0].piece.state() == Piece::State::Moving);

    // The caller's original is a detached copy — the arbiter owns its own.
    CHECK(original.state() == Piece::State::Idle);
}

TEST_CASE("inFlightPieces is empty with nothing in flight and shows a mover at its source") {
    RealTimeArbiter arbiter;
    CHECK(arbiter.inFlightPieces().empty());

    arbiter.startMotion(makePiece(Position{2, 0}), Position{2, 0}, Position{2, 3});
    std::vector<InFlightPiece> fliers = arbiter.inFlightPieces();
    REQUIRE(fliers.size() == 1);
    CHECK(fliers[0].cell == Position{2, 0});
}

TEST_CASE("inFlightPieces tracks a mover's real position as the clock advances, not just its source") {
    RealTimeArbiter arbiter;
    arbiter.startMotion(makePiece(Position{0, 0}), Position{0, 0}, Position{0, 2});

    CHECK(arbiter.inFlightPieces()[0].cell == Position{0, 0});
    arbiter.advanceTime(1000); // enters the intermediate cell, does not arrive yet
    CHECK(arbiter.inFlightPieces()[0].cell == Position{0, 1});
}

TEST_CASE("a move that has not reached its arrival time yet produces no events") {
    RealTimeArbiter arbiter;
    arbiter.startMotion(makePiece(Position{0, 0}), Position{0, 0}, Position{0, 1});
    ArrivalEvents events = arbiter.advanceTime(999);
    CHECK(events.empty());
    CHECK(arbiter.hasActiveMotion());
}

TEST_CASE("a one-cell move arrives after exactly 1000ms and hands the piece back") {
    RealTimeArbiter arbiter;
    arbiter.startMotion(makePiece(Position{0, 0}), Position{0, 0}, Position{0, 1});
    ArrivalEvents events = arbiter.advanceTime(1000);
    REQUIRE(events.size() == 1);
    CHECK(events[0].from == Position{0, 0});
    CHECK(events[0].to == Position{0, 1});
    CHECK_FALSE(events[0].intercepted);
    CHECK(events[0].piece.state() == Piece::State::Idle);
    CHECK(events[0].piece.cell() == Position{0, 1});
}

TEST_CASE("a two-cell move takes 2000ms") {
    RealTimeArbiter arbiter;
    arbiter.startMotion(makePiece(Position{0, 0}), Position{0, 0}, Position{0, 2});
    CHECK(arbiter.advanceTime(1000).empty());
    ArrivalEvents events = arbiter.advanceTime(1000);
    CHECK(events.size() == 1);
}

TEST_CASE("a diagonal move uses Chebyshev distance, not Euclidean distance") {
    RealTimeArbiter arbiter;
    arbiter.startMotion(makePiece(Position{0, 0}), Position{0, 0}, Position{3, 3});
    CHECK(arbiter.advanceTime(2999).empty());
    ArrivalEvents events = arbiter.advanceTime(1);
    CHECK(events.size() == 1);
}

TEST_CASE("hasActiveMotion becomes false and the motion is no longer tracked after arrival") {
    RealTimeArbiter arbiter;
    arbiter.startMotion(makePiece(Position{0, 0}), Position{0, 0}, Position{0, 1});
    arbiter.advanceTime(1000);
    CHECK_FALSE(arbiter.hasActiveMotion());
    CHECK(arbiter.inFlightPieces().empty());
}

TEST_CASE("a jump landing hands the piece back Idle at its own cell") {
    RealTimeArbiter arbiter;
    arbiter.startMotion(makePiece(Position{1, 1}), Position{1, 1}, Position{1, 1});
    ArrivalEvents events = arbiter.advanceTime(1000);
    REQUIRE(events.size() == 1);
    CHECK(events[0].from == Position{1, 1});
    CHECK(events[0].to == Position{1, 1});
    CHECK_FALSE(events[0].intercepted);
    CHECK(events[0].piece.state() == Piece::State::Idle);
}

TEST_CASE("hasJumpAt reports a jump's cell while airborne, and only a jump's") {
    RealTimeArbiter arbiter;
    arbiter.startMotion(makePiece(Position{1, 1}, 1), Position{1, 1}, Position{1, 1});
    arbiter.startMotion(makePiece(Position{0, 0}, 2), Position{0, 0}, Position{0, 2});

    CHECK(arbiter.hasJumpAt(Position{1, 1}));
    CHECK_FALSE(arbiter.hasJumpAt(Position{0, 0})); // a move's source is not a jump
    CHECK_FALSE(arbiter.hasJumpAt(Position{0, 2})); // a move's destination is not a jump

    arbiter.advanceTime(1000); // jump lands
    CHECK_FALSE(arbiter.hasJumpAt(Position{1, 1}));
}

TEST_CASE("an enemy move arriving at a cell with a still-active jump is intercepted") {
    RealTimeArbiter arbiter;
    arbiter.startMotion(makePiece(Position{1, 0}, 1), Position{1, 0}, Position{1, 0});   // airborne until t=1000
    arbiter.startMotion(makePiece(Position{1, 1}, 2, PieceColor::Black), Position{1, 1}, Position{1, 0}); // arrives at t=1000, same cell

    ArrivalEvents events = arbiter.advanceTime(1000);
    REQUIRE(events.size() == 2); // the jumper's landing and the attacker's interception

    const ArrivalEvent& jumperLanding = events[0];
    const ArrivalEvent& attackerArrival = events[1];
    CHECK_FALSE(jumperLanding.intercepted);
    CHECK(jumperLanding.piece.id() == 1);

    CHECK(attackerArrival.intercepted);
    CHECK(attackerArrival.piece.id() == 2);
    CHECK(attackerArrival.piece.state() == Piece::State::Captured);
}

TEST_CASE("a same-color piece arriving at a cell with a still-active jump is not intercepted") {
    RealTimeArbiter arbiter;
    arbiter.startMotion(makePiece(Position{1, 0}, 1), Position{1, 0}, Position{1, 0}); // airborne until t=1000
    arbiter.startMotion(makePiece(Position{1, 1}, 2), Position{1, 1}, Position{1, 0}); // same color, arrives at t=1000

    ArrivalEvents events = arbiter.advanceTime(1000);
    REQUIRE(events.size() == 2);
    CHECK_FALSE(events[0].intercepted);
    CHECK_FALSE(events[1].intercepted);
}

TEST_CASE("an enemy arriving after a jump has already landed captures normally, not intercepted") {
    RealTimeArbiter arbiter;
    arbiter.startMotion(makePiece(Position{1, 0}, 1), Position{1, 0}, Position{1, 0}); // airborne until t=1000
    arbiter.advanceTime(1000);                                                          // jump lands, no longer protected
    arbiter.startMotion(makePiece(Position{1, 3}, 2, PieceColor::Black), Position{1, 3}, Position{1, 0}); // distance 3: arrives at t=4000

    ArrivalEvents events = arbiter.advanceTime(3000);
    REQUIRE(events.size() == 1);
    CHECK_FALSE(events[0].intercepted);
}

TEST_CASE("two partial waits that sum to the full duration deliver the same arrival as one full wait") {
    RealTimeArbiter arbiter;
    arbiter.startMotion(makePiece(Position{0, 0}), Position{0, 0}, Position{0, 1});
    CHECK(arbiter.advanceTime(400).empty());
    ArrivalEvents events = arbiter.advanceTime(600);
    CHECK(events.size() == 1);
}
