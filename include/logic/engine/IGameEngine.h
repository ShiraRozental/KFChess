#pragma once
#include <optional>
#include <set>
#include <string>
#include "logic/engine/GameSnapshot.h"
#include "logic/model/PieceColor.h"
#include "logic/model/Position.h"

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

class IGameEngine {
public:
    virtual ~IGameEngine() = default;

    virtual MoveResult requestMove(const Position& source, const Position& destination) = 0;
    virtual void requestJump(const Position& cell) = 0;

    virtual bool hasPieceAt(const Position& pos) const = 0;
    virtual std::set<Position> legalDestinationsFrom(const Position& cell) const = 0;
    virtual void wait(int ms) = 0;
    virtual GameSnapshot snapshot() const = 0;

    virtual bool isGameOver() const = 0;
    virtual std::optional<PieceColor> winner() const = 0;
};
