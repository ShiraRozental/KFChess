#include "logic/io/BoardPrinter.h"
#include "logic/io/PieceNotation.h"

void printBoard(const GameSnapshot& snapshot, std::ostream& out) {
    const Board& board = snapshot.board();
    int rows = board.rowCount();
    int cols = board.colCount();

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            if (col > 0) out << " ";
            out << encodeCell(board.pieceAt(row, col));
        }
        if (row < rows - 1) out << "\n";
    }
}

void printParseError(const std::string& errorMessage, std::ostream& out) {
    out << errorMessage << "\n";
}
