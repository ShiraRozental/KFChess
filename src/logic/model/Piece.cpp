#include "logic/model/Piece.h"

Piece::Piece(PieceId id, PieceColor color, PieceType kind, Position cell)
    : id_(id), color_(color), kind_(kind), cell_(cell) {
}

PieceId Piece::id() const { return id_; }
PieceColor Piece::color() const { return color_; }
PieceType Piece::kind() const { return kind_; }
Position Piece::cell() const { return cell_; }
Piece::State Piece::state() const { return state_; }

void Piece::moveTo(Position cell) {
    cell_ = cell;
}

void Piece::setState(State state) {
    state_ = state;
}
