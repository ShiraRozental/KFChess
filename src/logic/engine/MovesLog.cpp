#include "logic/engine/MovesLog.h"
#include <algorithm>
#include "logic/io/PieceNotation.h"

MovesLog::MovesLog(int boardRowCount) : boardRowCount_(boardRowCount) {
}

void MovesLog::onMoveApplied(const MoveAppliedEvent& event) {
    std::string text = moveText(event.kind, event.to, event.isJump, boardRowCount_);
    entriesFor(event.color).push_back(MoveLogEntry{event.timestampMs, std::move(text)});
}

std::vector<MoveLogEntry> MovesLog::lastEntriesOf(PieceColor color, int maxCount) const {
    const std::vector<MoveLogEntry>& entries = entriesFor(color);
    int available = static_cast<int>(entries.size());
    int count = std::max(0, std::min(maxCount, available));
    return std::vector<MoveLogEntry>(entries.end() - count, entries.end());
}

const std::vector<MoveLogEntry>& MovesLog::entriesFor(PieceColor color) const {
    return color == PieceColor::White ? whiteEntries_ : blackEntries_;
}

std::vector<MoveLogEntry>& MovesLog::entriesFor(PieceColor color) {
    return color == PieceColor::White ? whiteEntries_ : blackEntries_;
}
