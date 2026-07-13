#include "io/BoardTextFormat.h"
#include "io/StringUtils.h"
#include "model/Piece.h"
#include <sstream>
#include <vector>
#include <optional>

namespace {
    const char kEmptyCellSymbol = '.';
    const char kWhiteColorSymbol = 'w';
    const char kBlackColorSymbol = 'b';
    const std::string kValidPieceLetters = "KQRBNP";

    // Returns true if the token has a valid chess-piece format.
    bool isValidToken(const std::string& token) {
        if (token.length() == 1 && token[0] == kEmptyCellSymbol) return true;
        if (token.length() != 2) return false;
        char color = token[0];
        char piece = token[1];
        if (color != kWhiteColorSymbol && color != kBlackColorSymbol) return false;
        return kValidPieceLetters.find(piece) != std::string::npos;
    }

    // Maps a token's piece letter to its PieceType. Returns nullopt for an
    // unrecognized letter (should not happen for tokens that already passed
    // isValidToken).
    std::optional<PieceType> charToPieceType(char c) {
        switch (c) {
            case 'K': return PieceType::King;
            case 'Q': return PieceType::Queen;
            case 'R': return PieceType::Rook;
            case 'B': return PieceType::Bishop;
            case 'N': return PieceType::Knight;
            case 'P': return PieceType::Pawn;
            default:  return std::nullopt;
        }
    }

    // Maps a token's color letter to its PieceColor. Returns nullopt for an
    // unrecognized letter (should not happen for tokens that already passed
    // isValidToken).
    std::optional<PieceColor> charToPieceColor(char c) {
        switch (c) {
            case kWhiteColorSymbol: return PieceColor::White;
            case kBlackColorSymbol: return PieceColor::Black;
            default:                return std::nullopt;
        }
    }

    // Maps a PieceType to its token letter. Inverse of charToPieceType.
    char pieceTypeToChar(PieceType type) {
        switch (type) {
            case PieceType::King:   return 'K';
            case PieceType::Queen:  return 'Q';
            case PieceType::Rook:   return 'R';
            case PieceType::Bishop: return 'B';
            case PieceType::Knight: return 'N';
            case PieceType::Pawn:   return 'P';
        }
        return 'Q'; // unreachable for valid enum values; keeps all paths returning
    }

    // Maps a PieceColor to its token letter. Inverse of charToPieceColor.
    char pieceColorToChar(PieceColor color) {
        return color == PieceColor::White ? kWhiteColorSymbol : kBlackColorSymbol;
    }
}

// Reads the board's text section into token rows, then builds a Board from
// them. Parsing (text -> tokens) and construction (tokens -> Piece objects
// placed on a Board) are kept as two passes so the second pass only runs
// once the whole section is known to be well-formed.
bool BoardTextFormat::parse(std::istream& in, Board& outBoard, std::string& errorMessage) {
    std::string line;
    bool readingBoard = false;
    std::vector<std::vector<std::string>> tokenRows;
    int width = -1;

    while (std::getline(in, line)) {
        std::string marker = trim(line);
        if (marker == "Board:") { readingBoard = true; continue; }
        if (marker == "Commands:") { readingBoard = false; continue; }

        if (!readingBoard || marker.empty()) continue;

        std::istringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;

        while (ss >> token) {
            if (!isValidToken(token)) {
                errorMessage = "ERROR UNKNOWN_TOKEN";
                return false;
            }
            tokens.push_back(token);
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
            if (token.length() == 1 && token[0] == kEmptyCellSymbol) continue;

            std::optional<PieceColor> color = charToPieceColor(token[0]);
            std::optional<PieceType> kind = charToPieceType(token[1]);
            built.addPiece(row, col, Piece(nextId, *color, *kind, Position{row, col}));
            ++nextId;
        }
    }

    outBoard = std::move(built);
    return true;
}

// Writes board as text purely through Board's public read API (pieceAt,
// rowCount, colCount) — this class never touches Board's internals.
void BoardTextFormat::write(const Board& board, std::ostream& out) {
    int rows = board.rowCount();
    int cols = board.colCount();

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            if (col > 0) out << " ";
            const Piece* piece = board.pieceAt(row, col);
            if (!piece) {
                out << kEmptyCellSymbol;
            } else {
                out << pieceColorToChar(piece->color()) << pieceTypeToChar(piece->kind());
            }
        }
        if (row < rows - 1) out << "\n";
    }
}
