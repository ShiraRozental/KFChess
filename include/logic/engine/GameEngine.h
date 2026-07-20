#pragma once
#include <optional>
#include <set>
#include <string>
#include <vector>
#include "logic/bus/GameEventListener.h"
#include "logic/engine/GameSnapshot.h"
#include "logic/engine/IGameEngine.h"
#include "logic/model/Board.h"
#include "logic/model/GameState.h"
#include "logic/model/PieceColor.h"
#include "logic/model/PieceType.h"
#include "logic/model/Position.h"
#include "logic/realtime/RealTimeArbiter.h"

// The application-service coordinator: the public command boundary used by
// Controller and ScriptRunner. Speaks only in board cells (Position) —
// no pixels, no text parsing, no rendering, no piece-specific movement
// logic (that's RuleEngine/PieceRules) — and delegates all motion timing
// to RealTimeArbiter.
class GameEngine : public IGameEngine {
public:
    explicit GameEngine(Board board = Board());

    MoveResult requestMove(const Position& source, const Position& destination) override;
    void requestJump(const Position& cell) override;

    bool hasPieceAt(const Position& pos) const override;
    std::set<Position> legalDestinationsFrom(const Position& cell) const override;
    void wait(int ms) override;
    GameSnapshot snapshot() const override;

    bool isGameOver() const override;
    std::optional<PieceColor> winner() const override;

    void addListener(GameEventListener& listener);

private:
    void notifyMoveApplied(const ArrivalEvent& event, bool wasJump);
    void notifyPieceCaptured(const Piece& captured, PieceColor capturedBy);
    void notifyGameStarted();
    void notifyGameEnded(PieceColor winner);
    void finishGame(PieceColor winner);
    void landPiece(Piece piece, Position cell, bool wasJump);
    void clearRestAt(Position cell, PieceId pieceId);
    void promoteIfNeeded(int row, int col, PieceType type, PieceColor color);
    static GameState winningStateFor(PieceColor color);
    static PieceColor opponentOf(PieceColor color);

    Board board_;
    RealTimeArbiter arbiter_;
    GameState gameState_ = GameState::InProgress;
    std::vector<GameEventListener*> listeners_;
    bool started_ = false;
};
