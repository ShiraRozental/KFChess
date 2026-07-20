#pragma once
#include <string>
#include <vector>
#include "logic/bus/GameEventListener.h"

struct MoveLogEntry {
    long long timestampMs;
    std::string text;
};

class MovesLog : public GameEventListener {
public:
    explicit MovesLog(int boardRowCount);

    void onMoveApplied(const MoveAppliedEvent& event) override;
    std::vector<MoveLogEntry> lastEntriesOf(PieceColor color, int maxCount) const;

private:
    const std::vector<MoveLogEntry>& entriesFor(PieceColor color) const;
    std::vector<MoveLogEntry>& entriesFor(PieceColor color);

    int boardRowCount_;
    std::vector<MoveLogEntry> whiteEntries_;
    std::vector<MoveLogEntry> blackEntries_;
};
