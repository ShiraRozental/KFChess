#include "doctest/doctest.h"
#include "realtime/RealTimeArbiter.h"
#include <algorithm>

namespace {
    Piece makePiece(Position cell, PieceId id = 1, PieceColor color = PieceColor::White) {
        return Piece(id, color, PieceType::Rook, cell);
    }
}

TEST_CASE("hasActiveMotion is false with nothing in flight") {
    RealTimeArbiter arbiter;
    CHECK_FALSE(arbiter.hasActiveMotion());
}

TEST_CASE("isCoolingDown is false for a piece with no cooldown started") {
    RealTimeArbiter arbiter;
    CHECK_FALSE(arbiter.isCoolingDown(1));
}

TEST_CASE("starting a cooldown makes isCoolingDown true for that piece id") {
    RealTimeArbiter arbiter;
    arbiter.startCooldown(1, Position{0, 0}, 1000);
    CHECK(arbiter.isCoolingDown(1));
}

TEST_CASE("isCoolingDown becomes false once advanceTime passes the cooldown's duration") {
    RealTimeArbiter arbiter;
    arbiter.startCooldown(1, Position{0, 0}, 1000);
    arbiter.advanceTime(999);
    CHECK(arbiter.isCoolingDown(1));
    arbiter.advanceTime(1);
    CHECK_FALSE(arbiter.isCoolingDown(1));
}

TEST_CASE("a cooldown for one piece id does not affect another") {
    RealTimeArbiter arbiter;
    arbiter.startCooldown(1, Position{0, 0}, 1000);
    CHECK_FALSE(arbiter.isCoolingDown(2));
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

TEST_CASE("startMotion sets a jump's in-flight state to Jumping, not Moving") {
    RealTimeArbiter arbiter;
    arbiter.startMotion(makePiece(Position{1, 1}), Position{1, 1}, Position{1, 1});
    std::vector<InFlightPiece> fliers = arbiter.inFlightPieces();
    REQUIRE(fliers.size() == 1);
    CHECK(fliers[0].piece.state() == Piece::State::Jumping);
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
    ArrivalEvents events = arbiter.advanceTime(999).arrivals;
    CHECK(events.empty());
    CHECK(arbiter.hasActiveMotion());
}

TEST_CASE("a one-cell move arrives after exactly 1000ms and hands the piece back") {
    RealTimeArbiter arbiter;
    arbiter.startMotion(makePiece(Position{0, 0}), Position{0, 0}, Position{0, 1});
    ArrivalEvents events = arbiter.advanceTime(1000).arrivals;
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
    CHECK(arbiter.advanceTime(1000).arrivals.empty());
    ArrivalEvents events = arbiter.advanceTime(1000).arrivals;
    CHECK(events.size() == 1);
}

TEST_CASE("a diagonal move uses Chebyshev distance, not Euclidean distance") {
    RealTimeArbiter arbiter;
    arbiter.startMotion(makePiece(Position{0, 0}), Position{0, 0}, Position{3, 3});
    CHECK(arbiter.advanceTime(2999).arrivals.empty());
    ArrivalEvents events = arbiter.advanceTime(1).arrivals;
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
    ArrivalEvents events = arbiter.advanceTime(1000).arrivals;
    REQUIRE(events.size() == 1);
    CHECK(events[0].from == Position{1, 1});
    CHECK(events[0].to == Position{1, 1});
    CHECK_FALSE(events[0].intercepted);
    CHECK(events[0].piece.state() == Piece::State::Idle);
}

TEST_CASE("hasMotionFrom reports a source cell for either a move or a jump") {
    RealTimeArbiter arbiter;
    arbiter.startMotion(makePiece(Position{1, 1}, 1), Position{1, 1}, Position{1, 1});
    arbiter.startMotion(makePiece(Position{0, 0}, 2), Position{0, 0}, Position{0, 2});

    CHECK(arbiter.hasMotionFrom(Position{1, 1}));
    CHECK(arbiter.hasMotionFrom(Position{0, 0}));
    CHECK_FALSE(arbiter.hasMotionFrom(Position{0, 2})); // a move's destination is not its source

    arbiter.advanceTime(1000);
    CHECK_FALSE(arbiter.hasMotionFrom(Position{1, 1})); // jump has landed, no longer tracked
}

TEST_CASE("an enemy move arriving at a cell with a still-active jump is intercepted") {
    RealTimeArbiter arbiter;
    arbiter.startMotion(makePiece(Position{1, 0}, 1), Position{1, 0}, Position{1, 0});   // airborne until t=1000
    arbiter.startMotion(makePiece(Position{1, 1}, 2, PieceColor::Black), Position{1, 1}, Position{1, 0}); // arrives at t=1000, same cell

    ArrivalEvents events = arbiter.advanceTime(1000).arrivals;
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
    arbiter.startMotion(makePiece(Position{1, 1}, 2), Position{1, 1}, Position{1, 0}); // same color, stopped short instead

    ArrivalEvents events = arbiter.advanceTime(1000).arrivals;
    REQUIRE(events.size() == 2);
    CHECK_FALSE(events[0].intercepted);
    CHECK_FALSE(events[1].intercepted);
    CHECK(events[1].to == Position{1, 1}); // never reached the jumper's cell — stopped at its own source
}

TEST_CASE("an enemy jump intercepts a mover even mid-path, not only at its final destination") {
    RealTimeArbiter arbiter;
    arbiter.startMotion(makePiece(Position{1, 1}, 1), Position{1, 1}, Position{1, 1}); // airborne until t=1000
    arbiter.startMotion(makePiece(Position{1, 0}, 2, PieceColor::Black), Position{1, 0}, Position{1, 3}); // passes through (1,1) at t=1000

    ArrivalEvents events = arbiter.advanceTime(1000).arrivals;
    REQUIRE(events.size() == 2);

    auto mover = std::find_if(events.begin(), events.end(), [](const ArrivalEvent& e) { return e.piece.id() == 2; });
    REQUIRE(mover != events.end());
    CHECK(mover->intercepted);
    CHECK(mover->to == Position{1, 1});
}

TEST_CASE("opposite-color paths crossing mid-flight: the later mover captures the earlier one") {
    RealTimeArbiter arbiter;
    arbiter.startMotion(makePiece(Position{0, 0}, 1), Position{0, 0}, Position{0, 4}); // enters (0,2) at t=2000
    // (0,2) is just a waypoint along this longer path, not its destination — isolates the capture from the winner's own later arrival.
    arbiter.startMotion(makePiece(Position{3, 2}, 2, PieceColor::Black), Position{3, 2}, Position{-2, 2}); // enters (0,2) at t=3000

    ArrivalEvents events = arbiter.advanceTime(3000).arrivals;
    REQUIRE(events.size() == 1);
    CHECK(events[0].piece.id() == 1); // the earlier mover, captured
    CHECK(events[0].intercepted);
    CHECK(events[0].piece.state() == Piece::State::Captured);
    CHECK(arbiter.hasActiveMotion()); // the later (winning) mover is still in flight, headed past (0,2)
}

TEST_CASE("same-color paths crossing mid-flight: the later mover stops one cell short (rook/queen example)") {
    RealTimeArbiter arbiter;
    // Queen-like: (3,7) -> (3,0), starts at t=0, enters (3,4) at t=3000.
    arbiter.startMotion(makePiece(Position{3, 7}, 1), Position{3, 7}, Position{3, 0});
    // Rook-like: starts later (t=1000), (0,4) -> (6,4), enters (3,4) at t=4000 — later than the queen.
    arbiter.advanceTime(1000);
    arbiter.startMotion(makePiece(Position{0, 4}, 2), Position{0, 4}, Position{6, 4});

    ArrivalEvents events = arbiter.advanceTime(3000).arrivals; // clock now at 4000
    REQUIRE(events.size() == 1);
    CHECK(events[0].piece.id() == 2); // the rook, stopped
    CHECK_FALSE(events[0].intercepted);
    CHECK(events[0].to == Position{2, 4}); // one cell short of (3,4)
}

TEST_CASE("a knight-shaped move that collides with a same-color piece at its destination never gets underway") {
    RealTimeArbiter arbiter;
    arbiter.startMotion(makePiece(Position{1, 1}, 1), Position{1, 1}, Position{2, 1}); // enters (2,1) at t=1000
    arbiter.startMotion(makePiece(Position{0, 0}, 2), Position{0, 0}, Position{2, 1}); // knight shape, enters (2,1) at t=2000

    ArrivalEvents events = arbiter.advanceTime(2000).arrivals;
    REQUIRE(events.size() == 2);

    auto knight = std::find_if(events.begin(), events.end(), [](const ArrivalEvent& e) { return e.piece.id() == 2; });
    REQUIRE(knight != events.end());
    CHECK_FALSE(knight->intercepted);
    CHECK(knight->to == Position{0, 0}); // no prior cell exists on a knight's path — it never left home
}

TEST_CASE("an enemy arriving after a jump has already landed captures normally, not intercepted") {
    RealTimeArbiter arbiter;
    arbiter.startMotion(makePiece(Position{1, 0}, 1), Position{1, 0}, Position{1, 0}); // airborne until t=1000
    arbiter.advanceTime(1000);                                                          // jump lands, no longer protected
    arbiter.startMotion(makePiece(Position{1, 3}, 2, PieceColor::Black), Position{1, 3}, Position{1, 0}); // distance 3: arrives at t=4000

    ArrivalEvents events = arbiter.advanceTime(3000).arrivals;
    REQUIRE(events.size() == 1);
    CHECK_FALSE(events[0].intercepted);
}

TEST_CASE("two partial waits that sum to the full duration deliver the same arrival as one full wait") {
    RealTimeArbiter arbiter;
    arbiter.startMotion(makePiece(Position{0, 0}), Position{0, 0}, Position{0, 1});
    CHECK(arbiter.advanceTime(400).arrivals.empty());
    ArrivalEvents events = arbiter.advanceTime(600).arrivals;
    CHECK(events.size() == 1);
}

TEST_CASE("a cooldown reports as expired exactly on the tick it elapses, not before") {
    RealTimeArbiter arbiter;
    arbiter.startCooldown(7, Position{2, 3}, 1000);
    CHECK(arbiter.advanceTime(999).expiredCooldowns.empty());

    std::vector<ExpiredCooldown> expired = arbiter.advanceTime(1).expiredCooldowns;
    REQUIRE(expired.size() == 1);
    CHECK(expired[0].pieceId == 7);
    CHECK(expired[0].cell == Position{2, 3});
}

TEST_CASE("an expired cooldown is reported once, not again on the next tick") {
    RealTimeArbiter arbiter;
    arbiter.startCooldown(7, Position{2, 3}, 1000);
    CHECK(arbiter.advanceTime(1000).expiredCooldowns.size() == 1);
    CHECK(arbiter.advanceTime(1000).expiredCooldowns.empty());
}

TEST_CASE("two cooldowns elapsing on the same tick are both reported") {
    RealTimeArbiter arbiter;
    arbiter.startCooldown(1, Position{0, 0}, 1000);
    arbiter.startCooldown(2, Position{1, 1}, 1000);
    CHECK(arbiter.advanceTime(1000).expiredCooldowns.size() == 2);
}
