#pragma once
#include "engine/GameEvents.h"

class GameEventListener {
public:
    virtual ~GameEventListener() = default;
    virtual void onMoveApplied(const MoveAppliedEvent&) {}
    virtual void onPieceCaptured(const PieceCapturedEvent&) {}
};
