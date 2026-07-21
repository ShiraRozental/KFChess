#include "logic/engine/ScoreBoard.h"
#include "logic/config/PieceCatalog.h"

void ScoreBoard::onPieceCaptured(const PieceCapturedEvent& event) {
    scoreFor(event.capturedBy) +=
        PieceCatalog::standard().definitionFor(event.capturedKind).captureCost;
}

int ScoreBoard::scoreOf(PieceColor color) const {
    return color == PieceColor::White ? whiteScore_ : blackScore_;
}

int& ScoreBoard::scoreFor(PieceColor color) {
    return color == PieceColor::White ? whiteScore_ : blackScore_;
}
