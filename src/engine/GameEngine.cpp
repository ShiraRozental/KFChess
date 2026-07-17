#include "engine/GameEngine.h"
#include "model/Piece.h"
#include "rules/RuleEngine.h"
#include "rules/LegalDestinations.h"
#include "rules/MovementRuleFactory.h"
#include "realtime/CooldownConfig.h"
#include <utility>

GameEngine::GameEngine(Board board) : board_(std::move(board)) {
}

MoveResult GameEngine::requestMove(const Position& source, const Position& destination) {
    if (isGameOver()) return MoveResult{false, MoveResultReason::GameOver};
    if (arbiter_.hasMotionFrom(source)) return MoveResult{false, MoveResultReason::MotionInProgress};

    const Piece* mover = board_.pieceAt(source.row, source.col);
    if (mover && arbiter_.isCoolingDown(mover->id())) {
        return MoveResult{false, MoveResultReason::CoolingDown};
    }

    MoveValidation validation = validateMove(board_, source.row, source.col, destination.row, destination.col);
    if (!validation.is_valid) return MoveResult{false, validation.reason};

    std::optional<Piece> movingPiece = board_.pieceCopyAt(source.row, source.col);
    board_.removePiece(source.row, source.col);
    arbiter_.startMotion(std::move(*movingPiece), source, destination);
    return MoveResult{true, MoveResultReason::Ok};
}

void GameEngine::requestJump(const Position& cell) {
    if (isGameOver()) return;

    std::optional<Piece> jumper = board_.pieceCopyAt(cell.row, cell.col);
    if (!jumper) return;
    if (arbiter_.isCoolingDown(jumper->id())) return;

    board_.removePiece(cell.row, cell.col);
    arbiter_.startMotion(std::move(*jumper), cell, cell);
}

bool GameEngine::hasPieceAt(const Position& pos) const {
    return board_.pieceAt(pos.row, pos.col) != nullptr;
}

std::set<Position> GameEngine::legalDestinationsFrom(const Position& cell) const {
    const Piece* piece = board_.pieceAt(cell.row, cell.col);
    if (!piece) return {};
    return legalDestinationsFor(board_, *piece);
}

// Advances simulated time and applies every motion RealTimeArbiter reports
// as arrived. An intercepted attacker never lands: its piece is simply
// discarded, leaving the defender (whose jump protected the cell)
// untouched. If a captured piece is a king, the game ends immediately;
// motions still due in this same batch are voided and their pieces return
// to where they took off from, so no piece ever vanishes from the board.
void GameEngine::wait(int ms) {
    if (isGameOver()) return;

    TimeStep step = arbiter_.advanceTime(ms);
    for (const ArrivalEvent& event : step.arrivals) {
        if (event.intercepted) {
            notifyPieceCaptured(event.piece, opponentOf(event.piece.color()));
            if (event.kingCaptured) {
                gameState_ = winningStateFor(opponentOf(event.piece.color()));
            }
            continue;
        }

        bool wasJump = event.wasJump;

        if (isGameOver()) {
            landPiece(event.piece, event.from, wasJump);
            continue;
        }

        std::optional<Piece> target = board_.pieceCopyAt(event.to.row, event.to.col);
        if (target && target->color() == event.piece.color()) {
            landPiece(event.piece, event.from, wasJump);
            continue;
        }

        bool capturesKing = target && target->kind() == PieceType::King;

        if (target) {
            notifyPieceCaptured(*target, event.piece.color());
        }
        board_.removePiece(event.to.row, event.to.col);
        landPiece(event.piece, event.to, wasJump);
        if (wasJump || event.from != event.to) {
            notifyMoveApplied(event, wasJump);
        }

        if (capturesKing) {
            gameState_ = winningStateFor(event.piece.color());
        }

        promoteIfNeeded(event.to.row, event.to.col, event.piece.kind(), event.piece.color());
    }

    for (const ExpiredCooldown& expired : step.expiredCooldowns) {
        clearRestAt(expired.cell, expired.pieceId);
    }
}

// The snapshot's board is a display composition: the settled board plus
// every in-flight piece shown at its current cell, so a piece mid-motion
// never "disappears" from a printed board or a renderer.
GameSnapshot GameEngine::snapshot() const {
    Board display = board_;
    std::map<PieceId, BoardPoint> inFlightPositions;
    for (const InFlightPiece& flier : arbiter_.inFlightPieces()) {
        display.addPiece(flier.cell.row, flier.cell.col, flier.piece);
        inFlightPositions.emplace(flier.piece.id(), flier.displayPosition);
    }
    return GameSnapshot(std::move(display), isGameOver(), winner(),
                        arbiter_.cooldownProgressByPiece(), std::move(inFlightPositions));
}

void GameEngine::addListener(GameEventListener& listener) {
    listeners_.push_back(&listener);
}

void GameEngine::notifyMoveApplied(const ArrivalEvent& event, bool wasJump) {
    MoveAppliedEvent moveEvent{event.piece.color(), event.piece.kind(),
                               event.from, event.to, wasJump, arbiter_.clockMs()};
    for (GameEventListener* listener : listeners_) {
        listener->onMoveApplied(moveEvent);
    }
}

void GameEngine::notifyPieceCaptured(const Piece& captured, PieceColor capturedBy) {
    PieceCapturedEvent captureEvent{captured.color(), captured.kind(),
                                    capturedBy, arbiter_.clockMs()};
    for (GameEventListener* listener : listeners_) {
        listener->onPieceCaptured(captureEvent);
    }
}

// Places a piece whose flight has resolved (normally, or voided by the game
// ending mid-batch) back onto the board at the given cell.
void GameEngine::landPiece(Piece piece, Position cell, bool wasJump) {
    piece.moveTo(cell);
    PieceId id = piece.id();
    PieceType kind = piece.kind();
    piece.setState(wasJump ? Piece::State::ShortRest : Piece::State::LongRest);
    board_.addPiece(cell.row, cell.col, std::move(piece));
    long long cooldownMs = wasJump ? jumpCooldownDurationMs() : cooldownDurationMsFor(kind);
    arbiter_.startCooldown(id, cell, cooldownMs);
}

void GameEngine::clearRestAt(Position cell, PieceId pieceId) {
    Piece* piece = board_.pieceAt(cell.row, cell.col);
    if (!piece || piece->id() != pieceId) return;
    if (piece->state() != Piece::State::ShortRest && piece->state() != Piece::State::LongRest) return;
    piece->setState(Piece::State::Idle);
}

// Promotes the piece now sitting at (row, col) to a queen if its movement
// rule reports that this row is far enough to promote (only pawns do).
// Piece type and color must be the ones the piece had before this move, so
// promotion is decided by the rule that actually made the move, not by
// whatever ends up on the board.
void GameEngine::promoteIfNeeded(int row, int col, PieceType type, PieceColor color) {
    const MovementRule& rule = movementRuleFor(type, color);
    if (rule.reachesPromotionRow(row, board_.rowCount())) {
        board_.promote(row, col, PieceType::Queen);
    }
}

// Returns true once a king has been captured and the game has a winner.
bool GameEngine::isGameOver() const {
    return gameState_ != GameState::InProgress;
}

// Returns the winning color, or nullopt while the game is still in progress.
std::optional<PieceColor> GameEngine::winner() const {
    if (gameState_ == GameState::WhiteWins) return PieceColor::White;
    if (gameState_ == GameState::BlackWins) return PieceColor::Black;
    return std::nullopt;
}

// Maps the color of the piece that captured the enemy king to the
// corresponding game outcome.
GameState GameEngine::winningStateFor(PieceColor color) {
    return color == PieceColor::White ? GameState::WhiteWins : GameState::BlackWins;
}

PieceColor GameEngine::opponentOf(PieceColor color) {
    return color == PieceColor::White ? PieceColor::Black : PieceColor::White;
}
