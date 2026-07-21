#pragma once
#include <map>
#include <optional>
#include <string>
#include "logic/engine/GameEngine.h"
#include "logic/engine/MovesLog.h"
#include "logic/engine/ScoreBoard.h"
#include "logic/model/Board.h"
#include "logic/model/PieceColor.h"
#include "logic/model/Position.h"
#include "server/session/IClientTransport.h"

enum class CommandOutcome {
    Accepted,
    UnknownConnection,
    Malformed,
    WrongColor,
    NotYourPiece,
    EngineRejected
};

class ServerSession {
public:
    ServerSession(Board board, IClientTransport& transport);

    std::optional<PieceColor> onConnected(ConnectionId connection, const std::string& username);
    CommandOutcome onCommand(ConnectionId connection, const std::string& text);
    void onDisconnected(ConnectionId connection);
    void tick(int ms);

    std::optional<PieceColor> colorOf(ConnectionId connection) const;
    std::optional<std::string> usernameOf(ConnectionId connection) const;

private:
    struct ConnectedPlayer {
        PieceColor color;
        std::string username;
    };

    void broadcastState();
    bool ownsPieceAt(PieceColor color, const Position& cell) const;

    int boardRowCount_;
    MovesLog movesLog_;
    GameEngine engine_;
    ScoreBoard scoreBoard_;
    IClientTransport& transport_;
    std::map<ConnectionId, ConnectedPlayer> players_;
    std::size_t assignedSeats_ = 0;
};
