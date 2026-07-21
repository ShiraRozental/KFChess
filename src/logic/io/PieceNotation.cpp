#include "logic/io/PieceNotation.h"

namespace {
    constexpr char kEmptyCellSymbol = '.';
    constexpr char kWhiteColorSymbol = 'w';
    constexpr char kBlackColorSymbol = 'b';
    constexpr char kFirstFileLetter = 'a';
    constexpr const char* kJumpMoveText = "Jump";
    std::optional<PieceColor> charToPieceColor(char c) {
        switch (c) {
            case kWhiteColorSymbol: return PieceColor::White;
            case kBlackColorSymbol: return PieceColor::Black;
            default:                return std::nullopt;
        }
    }

}

char pieceTypeToChar(PieceType type) {
    switch (type) {
        case PieceType::King:   return 'K';
        case PieceType::Queen:  return 'Q';
        case PieceType::Rook:   return 'R';
        case PieceType::Bishop: return 'B';
        case PieceType::Knight: return 'N';
        case PieceType::Pawn:   return 'P';
    }
    return 'Q';
}

char pieceColorToChar(PieceColor color) {
    return color == PieceColor::White ? kWhiteColorSymbol : kBlackColorSymbol;
}

std::optional<PieceType> pieceTypeFromChar(char c) {
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

bool isEmptyToken(const std::string& token) {
    return token.length() == 1 && token[0] == kEmptyCellSymbol;
}

std::optional<PieceCode> pieceFromToken(const std::string& token) {
    if (token.length() != 2) return std::nullopt;
    std::optional<PieceColor> color = charToPieceColor(token[0]);
    std::optional<PieceType> kind = pieceTypeFromChar(token[1]);
    if (!color || !kind) return std::nullopt;
    return PieceCode{*color, *kind};
}

std::string encodeCell(const Piece* piece) {
    if (!piece) return std::string(1, kEmptyCellSymbol);
    return std::string(1, pieceColorToChar(piece->color())) + pieceTypeToChar(piece->kind());
}

std::string fileLabel(int col) {
    return std::string(1, static_cast<char>(kFirstFileLetter + col));
}

std::string rankLabel(int row, int boardRowCount) {
    return std::to_string(boardRowCount - row);
}

std::string algebraicCell(const Position& cell, int boardRowCount) {
    return fileLabel(cell.col) + rankLabel(cell.row, boardRowCount);
}

std::optional<Position> parseAlgebraicCell(const std::string& text, int boardRowCount) {
    if (text.length() != 2) return std::nullopt;
    int col = text[0] - kFirstFileLetter;
    int rank = text[1] - '0';
    if (col < 0 || col >= boardRowCount) return std::nullopt;
    if (rank < 1 || rank > boardRowCount) return std::nullopt;
    return Position{boardRowCount - rank, col};
}

std::string moveText(PieceType kind, const Position& destination, bool isJump,
                     int boardRowCount) {
    if (isJump) return kJumpMoveText;
    std::string cellName = algebraicCell(destination, boardRowCount);
    if (kind == PieceType::Pawn) return cellName;
    return pieceTypeToChar(kind) + cellName;
}
