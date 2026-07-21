#include "shared/protocol/WireProtocol.h"
#include "logic/io/PieceNotation.h"

namespace {
    constexpr char kWhiteColorChar = 'W';
    constexpr char kBlackColorChar = 'B';
    constexpr std::size_t kMoveLength = 6;
    constexpr std::size_t kJumpLength = 4;
    constexpr std::size_t kColorIndex = 0;
    constexpr std::size_t kPieceIndex = 1;
    constexpr std::size_t kFirstCellIndex = 2;
    constexpr std::size_t kSecondCellIndex = 4;
    constexpr std::size_t kCellLength = 2;

    char colorToChar(PieceColor color) {
        return color == PieceColor::White ? kWhiteColorChar : kBlackColorChar;
    }

    std::optional<PieceColor> colorFromChar(char c) {
        if (c == kWhiteColorChar) return PieceColor::White;
        if (c == kBlackColorChar) return PieceColor::Black;
        return std::nullopt;
    }
}

bool operator==(const WireCommand& lhs, const WireCommand& rhs) {
    if (lhs.type != rhs.type || lhs.color != rhs.color || lhs.piece != rhs.piece) {
        return false;
    }
    if (lhs.from != rhs.from) return false;
    return lhs.type == WireCommandType::Jump || lhs.to == rhs.to;
}

std::string encodeMove(PieceColor color, PieceType piece,
                       const Position& from, const Position& to, int boardRowCount) {
    return std::string(1, colorToChar(color)) + pieceTypeToChar(piece) +
           algebraicCell(from, boardRowCount) + algebraicCell(to, boardRowCount);
}

std::string encodeJump(PieceColor color, PieceType piece,
                       const Position& cell, int boardRowCount) {
    return std::string(1, colorToChar(color)) + pieceTypeToChar(piece) +
           algebraicCell(cell, boardRowCount);
}

std::optional<WireCommand> parseCommand(const std::string& text, int boardRowCount) {
    if (text.length() != kMoveLength && text.length() != kJumpLength) return std::nullopt;

    std::optional<PieceColor> color = colorFromChar(text[kColorIndex]);
    std::optional<PieceType> piece = pieceTypeFromChar(text[kPieceIndex]);
    std::optional<Position> from =
        parseAlgebraicCell(text.substr(kFirstCellIndex, kCellLength), boardRowCount);
    if (!color || !piece || !from) return std::nullopt;

    if (text.length() == kJumpLength) {
        return WireCommand{WireCommandType::Jump, *color, *piece, *from, *from};
    }

    std::optional<Position> to =
        parseAlgebraicCell(text.substr(kSecondCellIndex, kCellLength), boardRowCount);
    if (!to) return std::nullopt;
    return WireCommand{WireCommandType::Move, *color, *piece, *from, *to};
}
