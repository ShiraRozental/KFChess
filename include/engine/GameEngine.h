#pragma once
#include <string>
#include <optional>
#include "engine/GameSnapshot.h"
#include "model/Board.h"
#include "model/GameState.h"
#include "model/PieceColor.h"
#include "model/PieceType.h"
#include "model/Position.h"
#include "realtime/RealTimeArbiter.h"

// Result of a requestMove call: whether it was accepted, and a stable
// reason — "ok" when accepted, "game_over"/"motion_in_progress" for
// application-level rejections, or the rule-level reason copied straight
// through from RuleEngine's MoveValidation.
struct MoveResult {
    bool is_accepted;
    std::string reason;
};

namespace MoveResultReason {
    constexpr const char* Ok = "ok";
    constexpr const char* GameOver = "game_over";
    constexpr const char* MotionInProgress = "motion_in_progress";
    constexpr const char* CoolingDown = "cooling_down";
}

// The application-service coordinator: the public command boundary used by
// Controller and ScriptRunner. Speaks only in board cells (Position) —
// no pixels, no text parsing, no rendering, no piece-specific movement
// logic (that's RuleEngine/PieceRules) — and delegates all motion timing
// to RealTimeArbiter.
class GameEngine {
public:
    explicit GameEngine(Board board = Board());

    MoveResult requestMove(const Position& source, const Position& destination);
    void requestJump(const Position& cell);

    bool hasPieceAt(const Position& pos) const;
    void wait(int ms);
    GameSnapshot snapshot() const;

    bool isGameOver() const;
    std::optional<PieceColor> winner() const;

private:
    void landPiece(Piece piece, Position cell, bool wasJump);
    void clearRestAt(Position cell, PieceId pieceId);
    void promoteIfNeeded(int row, int col, PieceType type, PieceColor color);
    static GameState winningStateFor(PieceColor color);
    static PieceColor opponentOf(PieceColor color);

    Board board_;
    RealTimeArbiter arbiter_;
    GameState gameState_ = GameState::InProgress;
};
