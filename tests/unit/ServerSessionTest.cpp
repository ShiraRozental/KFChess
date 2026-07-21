#include "doctest/doctest.h"
#include <sstream>
#include <utility>
#include <vector>
#include "logic/io/BoardParser.h"
#include "server/session/ServerSession.h"
#include "shared/protocol/GameStateCodec.h"
#include "shared/protocol/WireProtocol.h"

namespace {
    constexpr int kRows = 3;
    constexpr ConnectionId kFirst = 10;
    constexpr ConnectionId kSecond = 20;
    constexpr ConnectionId kThird = 30;
    constexpr ConnectionId kFourth = 40;

    const Position whiteRookCell{0, 0};
    const Position blackRookCell{0, 2};
    const Position emptyCell{1, 1};
    const Position whiteRookTarget{1, 0};

    const std::string kBoardText =
        "Board:\n"
        "wR . bR\n"
        ". . .\n"
        ". . .\n";

    class FakeTransport : public IClientTransport {
    public:
        void sendTo(ConnectionId connection, const std::string& text) override {
            sent.push_back({connection, text});
        }

        std::vector<std::string> textsTo(ConnectionId connection) const {
            std::vector<std::string> texts;
            for (const auto& entry : sent) {
                if (entry.first == connection) texts.push_back(entry.second);
            }
            return texts;
        }

        std::vector<std::pair<ConnectionId, std::string>> sent;
    };

    Board parseBoardText() {
        std::istringstream in(kBoardText);
        ParsedInput parsed;
        std::string errorMessage;
        REQUIRE(BoardParser::parse(in, parsed, errorMessage));
        return std::move(parsed.board);
    }

    std::optional<GameStateMessage> lastMessageTo(const FakeTransport& transport,
                                                  ConnectionId connection) {
        std::vector<std::string> texts = transport.textsTo(connection);
        if (texts.empty()) return std::nullopt;
        return parseGameState(texts.back());
    }

    std::string moveCommand(PieceColor color, PieceType piece,
                            const Position& from, const Position& to) {
        return encodeMove(color, piece, from, to, kRows);
    }

    std::string jumpCommand(PieceColor color, PieceType piece, const Position& cell) {
        return encodeJump(color, piece, cell, kRows);
    }

    void advance(ServerSession& session, int totalMs, int stepMs = 100) {
        for (int elapsed = 0; elapsed < totalMs; elapsed += stepMs) {
            session.tick(stepMs);
        }
    }
}

TEST_CASE("the first connection is seated as White") {
    FakeTransport transport;
    ServerSession session(parseBoardText(), transport);

    CHECK(session.onConnected(kFirst, "ann") == std::optional<PieceColor>{PieceColor::White});
}

TEST_CASE("the second connection is seated as Black") {
    FakeTransport transport;
    ServerSession session(parseBoardText(), transport);
    session.onConnected(kFirst, "ann");

    CHECK(session.onConnected(kSecond, "bob") == std::optional<PieceColor>{PieceColor::Black});
}

TEST_CASE("a third connection is rejected") {
    FakeTransport transport;
    ServerSession session(parseBoardText(), transport);
    session.onConnected(kFirst, "ann");
    session.onConnected(kSecond, "bob");

    CHECK_FALSE(session.onConnected(kThird, "cid").has_value());
    CHECK_FALSE(session.onConnected(kFourth, "dee").has_value());
}

TEST_CASE("a rejected third connection leaves both seated players working") {
    FakeTransport transport;
    ServerSession session(parseBoardText(), transport);
    session.onConnected(kFirst, "ann");
    session.onConnected(kSecond, "bob");
    session.onConnected(kThird, "cid");

    CHECK(session.colorOf(kFirst) == std::optional<PieceColor>{PieceColor::White});
    CHECK(session.colorOf(kSecond) == std::optional<PieceColor>{PieceColor::Black});
    CHECK(session.onCommand(kFirst, moveCommand(PieceColor::White, PieceType::Rook,
                                                whiteRookCell, whiteRookTarget))
          == CommandOutcome::Accepted);
}

TEST_CASE("connecting twice with the same connection id is rejected") {
    FakeTransport transport;
    ServerSession session(parseBoardText(), transport);
    session.onConnected(kFirst, "ann");

    CHECK_FALSE(session.onConnected(kFirst, "impostor").has_value());
    CHECK(session.usernameOf(kFirst) == std::optional<std::string>{"ann"});
}

TEST_CASE("the username given at connect time is kept") {
    FakeTransport transport;
    ServerSession session(parseBoardText(), transport);
    session.onConnected(kFirst, "ann");

    CHECK(session.usernameOf(kFirst) == std::optional<std::string>{"ann"});
    CHECK_FALSE(session.usernameOf(kSecond).has_value());
}

TEST_CASE("an unknown connection has no color") {
    FakeTransport transport;
    ServerSession session(parseBoardText(), transport);

    CHECK_FALSE(session.colorOf(kFirst).has_value());
}

TEST_CASE("a legal move from the owning color is accepted") {
    FakeTransport transport;
    ServerSession session(parseBoardText(), transport);
    session.onConnected(kFirst, "ann");

    CHECK(session.onCommand(kFirst, moveCommand(PieceColor::White, PieceType::Rook,
                                                whiteRookCell, whiteRookTarget))
          == CommandOutcome::Accepted);
}

TEST_CASE("a command tagged with the other color is rejected") {
    FakeTransport transport;
    ServerSession session(parseBoardText(), transport);
    session.onConnected(kFirst, "ann");

    CHECK(session.onCommand(kFirst, moveCommand(PieceColor::Black, PieceType::Rook,
                                                blackRookCell, emptyCell))
          == CommandOutcome::WrongColor);
}

TEST_CASE("a command claiming an opponent's piece as your own is rejected") {
    FakeTransport transport;
    ServerSession session(parseBoardText(), transport);
    session.onConnected(kFirst, "ann");

    CHECK(session.onCommand(kFirst, moveCommand(PieceColor::White, PieceType::Rook,
                                                blackRookCell, emptyCell))
          == CommandOutcome::NotYourPiece);
}

TEST_CASE("a command from an empty cell is rejected") {
    FakeTransport transport;
    ServerSession session(parseBoardText(), transport);
    session.onConnected(kFirst, "ann");

    CHECK(session.onCommand(kFirst, moveCommand(PieceColor::White, PieceType::Rook,
                                                emptyCell, whiteRookTarget))
          == CommandOutcome::NotYourPiece);
}

TEST_CASE("malformed command text is rejected") {
    FakeTransport transport;
    ServerSession session(parseBoardText(), transport);
    session.onConnected(kFirst, "ann");

    CHECK(session.onCommand(kFirst, "nonsense") == CommandOutcome::Malformed);
    CHECK(session.onCommand(kFirst, "") == CommandOutcome::Malformed);
}

TEST_CASE("a command from a connection that never joined is rejected") {
    FakeTransport transport;
    ServerSession session(parseBoardText(), transport);

    CHECK(session.onCommand(kFirst, moveCommand(PieceColor::White, PieceType::Rook,
                                                whiteRookCell, whiteRookTarget))
          == CommandOutcome::UnknownConnection);
}

TEST_CASE("a command from a disconnected connection is rejected") {
    FakeTransport transport;
    ServerSession session(parseBoardText(), transport);
    session.onConnected(kFirst, "ann");
    session.onDisconnected(kFirst);

    CHECK(session.onCommand(kFirst, moveCommand(PieceColor::White, PieceType::Rook,
                                                whiteRookCell, whiteRookTarget))
          == CommandOutcome::UnknownConnection);
}

TEST_CASE("a disconnect does not free the seat for a new connection") {
    FakeTransport transport;
    ServerSession session(parseBoardText(), transport);
    session.onConnected(kFirst, "ann");
    session.onConnected(kSecond, "bob");
    session.onDisconnected(kFirst);

    CHECK_FALSE(session.onConnected(kThird, "cid").has_value());
}

TEST_CASE("a jump on your own piece is accepted") {
    FakeTransport transport;
    ServerSession session(parseBoardText(), transport);
    session.onConnected(kFirst, "ann");

    CHECK(session.onCommand(kFirst, jumpCommand(PieceColor::White, PieceType::Rook,
                                                whiteRookCell))
          == CommandOutcome::Accepted);
}

TEST_CASE("a jump on an opponent's piece is rejected") {
    FakeTransport transport;
    ServerSession session(parseBoardText(), transport);
    session.onConnected(kFirst, "ann");

    CHECK(session.onCommand(kFirst, jumpCommand(PieceColor::White, PieceType::Rook,
                                                blackRookCell))
          == CommandOutcome::NotYourPiece);
}

TEST_CASE("a move the engine refuses is reported as an engine rejection") {
    FakeTransport transport;
    ServerSession session(parseBoardText(), transport);
    session.onConnected(kFirst, "ann");

    CHECK(session.onCommand(kFirst, moveCommand(PieceColor::White, PieceType::Rook,
                                                whiteRookCell, emptyCell))
          == CommandOutcome::EngineRejected);
}

TEST_CASE("the declared piece type is not validated against the piece on the board") {
    FakeTransport transport;
    ServerSession session(parseBoardText(), transport);
    session.onConnected(kFirst, "ann");

    CHECK(session.onCommand(kFirst, moveCommand(PieceColor::White, PieceType::Queen,
                                                whiteRookCell, whiteRookTarget))
          == CommandOutcome::Accepted);
}

TEST_CASE("a jump the engine refuses still reports Accepted, because requestJump returns void") {
    FakeTransport transport;
    ServerSession session(parseBoardText(), transport);
    session.onConnected(kFirst, "ann");
    session.onCommand(kFirst, jumpCommand(PieceColor::White, PieceType::Rook, whiteRookCell));
    advance(session, 1000);

    CHECK(session.onCommand(kFirst, jumpCommand(PieceColor::White, PieceType::Rook,
                                                whiteRookCell))
          == CommandOutcome::Accepted);

    session.tick(0);
    std::optional<GameStateMessage> state = lastMessageTo(transport, kFirst);
    REQUIRE(state.has_value());
    const Piece* rook = state->snapshot.board().pieceAt(whiteRookCell.row, whiteRookCell.col);
    REQUIRE(rook != nullptr);
    CHECK(rook->state() == Piece::State::ShortRest);
}

TEST_CASE("a tick sends one state message to every seated player") {
    FakeTransport transport;
    ServerSession session(parseBoardText(), transport);
    session.onConnected(kFirst, "ann");
    session.onConnected(kSecond, "bob");

    session.tick(0);

    CHECK(transport.textsTo(kFirst).size() == 1);
    CHECK(transport.textsTo(kSecond).size() == 1);
}

TEST_CASE("the broadcast message is decodable game state") {
    FakeTransport transport;
    ServerSession session(parseBoardText(), transport);
    session.onConnected(kFirst, "ann");

    session.tick(0);

    std::optional<GameStateMessage> state = lastMessageTo(transport, kFirst);
    REQUIRE(state.has_value());
    CHECK(state->snapshot.board().rowCount() == kRows);
    REQUIRE(state->snapshot.board().pieceAt(whiteRookCell.row, whiteRookCell.col) != nullptr);
    CHECK(state->snapshot.board().pieceAt(whiteRookCell.row, whiteRookCell.col)->color()
          == PieceColor::White);
}

TEST_CASE("a tick with nobody connected sends nothing") {
    FakeTransport transport;
    ServerSession session(parseBoardText(), transport);

    session.tick(0);

    CHECK(transport.sent.empty());
}

TEST_CASE("after a disconnect only the remaining player is broadcast to") {
    FakeTransport transport;
    ServerSession session(parseBoardText(), transport);
    session.onConnected(kFirst, "ann");
    session.onConnected(kSecond, "bob");
    session.onDisconnected(kFirst);

    session.tick(0);

    CHECK(transport.textsTo(kFirst).empty());
    CHECK(transport.textsTo(kSecond).size() == 1);
}

TEST_CASE("a completed move shows up on the broadcast board") {
    FakeTransport transport;
    ServerSession session(parseBoardText(), transport);
    session.onConnected(kFirst, "ann");
    session.onCommand(kFirst, moveCommand(PieceColor::White, PieceType::Rook,
                                          whiteRookCell, whiteRookTarget));
    advance(session, 1000);

    std::optional<GameStateMessage> state = lastMessageTo(transport, kFirst);
    REQUIRE(state.has_value());
    CHECK(state->snapshot.board().pieceAt(whiteRookCell.row, whiteRookCell.col) == nullptr);
    REQUIRE(state->snapshot.board().pieceAt(whiteRookTarget.row, whiteRookTarget.col) != nullptr);
}

TEST_CASE("the broadcast move log carries notation built from the real board height") {
    FakeTransport transport;
    ServerSession session(parseBoardText(), transport);
    session.onConnected(kFirst, "ann");
    session.onCommand(kFirst, moveCommand(PieceColor::White, PieceType::Rook,
                                          whiteRookCell, whiteRookTarget));
    advance(session, 1000);

    std::optional<GameStateMessage> state = lastMessageTo(transport, kFirst);
    REQUIRE(state.has_value());
    REQUIRE(state->whiteMoves.size() == 1);
    CHECK(state->whiteMoves[0].text == "Ra2");
    CHECK(state->blackMoves.empty());
}

TEST_CASE("a capture shows up as score on the broadcast") {
    FakeTransport transport;
    ServerSession session(parseBoardText(), transport);
    session.onConnected(kFirst, "ann");
    session.onCommand(kFirst, moveCommand(PieceColor::White, PieceType::Rook,
                                          whiteRookCell, blackRookCell));
    advance(session, 3000);

    std::optional<GameStateMessage> state = lastMessageTo(transport, kFirst);
    REQUIRE(state.has_value());
    CHECK(state->whiteScore > 0);
    CHECK(state->blackScore == 0);
}
