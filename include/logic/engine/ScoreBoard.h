#pragma once
#include "logic/bus/GameEventListener.h"

class ScoreBoard : public GameEventListener {
public:
    void onPieceCaptured(const PieceCapturedEvent& event) override;
    int scoreOf(PieceColor color) const;

private:
    int& scoreFor(PieceColor color);

    int whiteScore_ = 0;
    int blackScore_ = 0;
};
