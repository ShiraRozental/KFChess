#pragma once
#include <istream>
#include <ostream>
#include <string>
#include "model/Board.h"

// Reads and writes the board's text notation (e.g. "wP", "bK", ".") — the
// only place in the codebase that knows this grammar. Board itself has no
// idea text exists; a future BoardBinaryFormat could implement the same
// responsibility for a binary encoding without Board changing at all.
class BoardTextFormat {
public:
   
    static bool parse(std::istream& in, Board& outBoard, std::string& errorMessage);

    static void write(const Board& board, std::ostream& out);
};
