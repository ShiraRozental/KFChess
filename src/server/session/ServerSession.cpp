#include "server/session/ServerSession.h"
#include <array>
#include "shared/protocol/GameStateCodec.h"
#include "shared/protocol/WireProtocol.h"

namespace {
    constexpr std::array<PieceColor, 2> kSeatColors{PieceColor::White, PieceColor::Black};
    constexpr int kBroadcastLogEntries = 20;
}

ServerSession::ServerSession(Board board, IClientTransport& transport)
    : boardRowCount_(board.rowCount()), movesLog_(board.rowCount()), engine_(board),
      transport_(transport) {
    engine_.addListener(movesLog_);
    engine_.addListener(scoreBoard_);
}

std::optional<PieceColor> ServerSession::onConnected(ConnectionId connection,
                                                     const std::string& username) {
    if (players_.count(connection)) return std::nullopt;
    if (assignedSeats_ >= kSeatColors.size()) return std::nullopt;

    PieceColor color = kSeatColors[assignedSeats_];
    ++assignedSeats_;
    players_.emplace(connection, ConnectedPlayer{color, username});
    return color;
}

CommandOutcome ServerSession::onCommand(ConnectionId connection, const std::string& text) {
    std::map<ConnectionId, ConnectedPlayer>::const_iterator player = players_.find(connection);
    if (player == players_.end()) return CommandOutcome::UnknownConnection;

    std::optional<WireCommand> command = parseCommand(text, boardRowCount_);
    if (!command) return CommandOutcome::Malformed;
    if (command->color != player->second.color) return CommandOutcome::WrongColor;
    if (!ownsPieceAt(player->second.color, command->from)) return CommandOutcome::NotYourPiece;

    if (command->type == WireCommandType::Jump) {
        engine_.requestJump(command->from);
        return CommandOutcome::Accepted;
    }

    MoveResult result = engine_.requestMove(command->from, command->to);
    return result.is_accepted ? CommandOutcome::Accepted : CommandOutcome::EngineRejected;
}

void ServerSession::onDisconnected(ConnectionId connection) {
    players_.erase(connection);
}

void ServerSession::tick(int ms) {
    engine_.wait(ms);
    broadcastState();
}

std::optional<PieceColor> ServerSession::colorOf(ConnectionId connection) const {
    std::map<ConnectionId, ConnectedPlayer>::const_iterator player = players_.find(connection);
    if (player == players_.end()) return std::nullopt;
    return player->second.color;
}

std::optional<std::string> ServerSession::usernameOf(ConnectionId connection) const {
    std::map<ConnectionId, ConnectedPlayer>::const_iterator player = players_.find(connection);
    if (player == players_.end()) return std::nullopt;
    return player->second.username;
}

void ServerSession::broadcastState() {
    if (players_.empty()) return;

    GameStateMessage message{engine_.snapshot(),
                             movesLog_.lastEntriesOf(PieceColor::White, kBroadcastLogEntries),
                             movesLog_.lastEntriesOf(PieceColor::Black, kBroadcastLogEntries),
                             scoreBoard_.scoreOf(PieceColor::White),
                             scoreBoard_.scoreOf(PieceColor::Black)};
    std::string text = encodeGameState(message);
    for (const std::pair<const ConnectionId, ConnectedPlayer>& player : players_) {
        transport_.sendTo(player.first, text);
    }
}

bool ServerSession::ownsPieceAt(PieceColor color, const Position& cell) const {
    GameSnapshot snapshot = engine_.snapshot();
    const Piece* piece = snapshot.board().pieceAt(cell.row, cell.col);
    return piece != nullptr && piece->color() == color;
}
