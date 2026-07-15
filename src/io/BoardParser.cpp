#include "io/BoardParser.h"
#include "io/PieceNotation.h"
#include "io/StringUtils.h"
#include "model/Piece.h"

bool BoardParser::parse(std::istream& in, ParsedInput& out, std::string& errorMessage) {
    std::string line;
    bool readingCommands = false;
    std::vector<std::vector<std::string>> tokenRows;
    int width = -1;
    std::vector<std::string> commands;

    while (std::getline(in, line)) {
        std::string marker = trim(line);
        if (marker == "Board:") { readingCommands = false; continue; }
        if (marker == "Commands:") { readingCommands = true; continue; }

        if (readingCommands) {
            if (!marker.empty()) commands.push_back(line);
            continue;
        }

        if (marker.empty()) continue;

        std::vector<std::string> tokens = tokenize(line);
        for (const std::string& token : tokens) {
            if (!isEmptyToken(token) && !pieceFromToken(token).has_value()) {
                errorMessage = "ERROR UNKNOWN_TOKEN";
                return false;
            }
        }

        int currentWidth = (int)tokens.size();
        if (width == -1) {
            width = currentWidth;
        } else if (currentWidth != width) {
            errorMessage = "ERROR ROW_WIDTH_MISMATCH";
            return false;
        }

        tokenRows.push_back(std::move(tokens));
    }

    int rows = (int)tokenRows.size();
    int cols = (width == -1) ? 0 : width;
    Board built(rows, cols);

    PieceId nextId = 0;
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            const std::string& token = tokenRows[row][col];
            if (isEmptyToken(token)) continue;
            PieceCode code = *pieceFromToken(token);
            built.addPiece(row, col, Piece(nextId, code.color, code.kind, Position{row, col}));
            ++nextId;
        }
    }

    out.board = std::move(built);
    out.commands = std::move(commands);
    return true;
}
