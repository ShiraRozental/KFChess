#include "logic/engine/ScoreBoard.h"
#include "logic/model/PieceValues.h"

void ScoreBoard::onPieceCaptured(const PieceCapturedEvent& event) {
    scoreFor(event.capturedBy) += pieceCostOf(event.capturedKind);
}

int ScoreBoard::scoreOf(PieceColor color) const {
    return color == PieceColor::White ? whiteScore_ : blackScore_;
}

int& ScoreBoard::scoreFor(PieceColor color) {
    return color == PieceColor::White ? whiteScore_ : blackScore_;
}
