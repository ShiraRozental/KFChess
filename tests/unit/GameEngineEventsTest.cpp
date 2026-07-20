#include "doctest/doctest.h"
#include "engine/GameEngine.h"
#include "io/BoardParser.h"
#include <sstream>
#include <vector>

namespace {
    GameEngine makeGame(const std::string& boardText) {
        std::istringstream in(boardText);
        ParsedInput parsed;
        std::string error;
        BoardParser::parse(in, parsed, error);
        return GameEngine(std::move(parsed.board));
    }

    struct RecordingListener : GameEventListener {
        std::vector<MoveAppliedEvent> moves;
        std::vector<PieceCapturedEvent> captures;
        std::vector<GameStartedEvent> gameStarted;
        std::vector<GameEndedEvent> gameEnded;

        void onMoveApplied(const MoveAppliedEvent& event) override { moves.push_back(event); }
        void onPieceCaptured(const PieceCapturedEvent& event) override { captures.push_back(event); }
        void onGameStarted(const GameStartedEvent& event) override { gameStarted.push_back(event); }
        void onGameEnded(const GameEndedEvent& event) override { gameEnded.push_back(event); }
    };
}

TEST_CASE("a completed move notifies onMoveApplied with the move's facts") {
    GameEngine game = makeGame("Board:\nwR . .\n");
    RecordingListener listener;
    game.addListener(listener);

    game.requestMove(Position{0, 0}, Position{0, 2});
    game.wait(2000);

    REQUIRE(listener.moves.size() == 1);
    CHECK(listener.moves[0].color == PieceColor::White);
    CHECK(listener.moves[0].kind == PieceType::Rook);
    CHECK(listener.moves[0].from == Position{0, 0});
    CHECK(listener.moves[0].to == Position{0, 2});
    CHECK_FALSE(listener.moves[0].isJump);
    CHECK(listener.captures.empty());
}

TEST_CASE("a landed jump notifies onMoveApplied with isJump set") {
    GameEngine game = makeGame("Board:\nwK . .\n");
    RecordingListener listener;
    game.addListener(listener);

    game.requestJump(Position{0, 0});
    game.wait(1000);

    REQUIRE(listener.moves.size() == 1);
    CHECK(listener.moves[0].isJump);
    CHECK(listener.moves[0].from == Position{0, 0});
    CHECK(listener.moves[0].to == Position{0, 0});
}

TEST_CASE("a capture on landing notifies onPieceCaptured before onMoveApplied") {
    GameEngine game = makeGame("Board:\nwR . bN\n");
    RecordingListener listener;
    game.addListener(listener);

    game.requestMove(Position{0, 0}, Position{0, 2});
    game.wait(2000);

    REQUIRE(listener.captures.size() == 1);
    CHECK(listener.captures[0].capturedColor == PieceColor::Black);
    CHECK(listener.captures[0].capturedKind == PieceType::Knight);
    CHECK(listener.captures[0].capturedBy == PieceColor::White);
    REQUIRE(listener.moves.size() == 1);
    CHECK(listener.captures[0].timestampMs <= listener.moves[0].timestampMs);
}

TEST_CASE("a mid-flight interception notifies onPieceCaptured without onMoveApplied for the victim") {
    GameEngine game = makeGame("Board:\nwK bR .\n");
    RecordingListener listener;
    game.addListener(listener);

    game.requestJump(Position{0, 0});
    game.requestMove(Position{0, 1}, Position{0, 0});
    game.wait(1000);

    REQUIRE(listener.captures.size() == 1);
    CHECK(listener.captures[0].capturedColor == PieceColor::Black);
    CHECK(listener.captures[0].capturedKind == PieceType::Rook);
    CHECK(listener.captures[0].capturedBy == PieceColor::White);
    REQUIRE(listener.moves.size() == 1);
    CHECK(listener.moves[0].isJump);
}

TEST_CASE("a piece bounced back to its source notifies no events") {
    GameEngine game = makeGame("Board:\nwR . wK\n");
    RecordingListener listener;
    game.addListener(listener);

    game.requestMove(Position{0, 0}, Position{0, 1});
    game.requestMove(Position{0, 2}, Position{0, 1});
    game.wait(2000);

    REQUIRE(listener.moves.size() == 1);
    CHECK(listener.moves[0].from == Position{0, 0});
    CHECK(listener.captures.empty());
}

TEST_CASE("every registered listener is notified") {
    GameEngine game = makeGame("Board:\nwR . .\n");
    RecordingListener first;
    RecordingListener second;
    game.addListener(first);
    game.addListener(second);

    game.requestMove(Position{0, 0}, Position{0, 1});
    game.wait(1000);

    CHECK(first.moves.size() == 1);
    CHECK(second.moves.size() == 1);
}

TEST_CASE("event timestamps reflect the accumulated game clock") {
    GameEngine game = makeGame("Board:\nwR . .\n");
    RecordingListener listener;
    game.addListener(listener);

    game.requestMove(Position{0, 0}, Position{0, 1});
    game.wait(300);
    game.wait(700);

    REQUIRE(listener.moves.size() == 1);
    CHECK(listener.moves[0].timestampMs == 1000);
}

TEST_CASE("capturing a king notifies onPieceCaptured and the game ends") {
    GameEngine game = makeGame("Board:\nwR bK\n");
    RecordingListener listener;
    game.addListener(listener);

    game.requestMove(Position{0, 0}, Position{0, 1});
    game.wait(1000);

    REQUIRE(listener.captures.size() == 1);
    CHECK(listener.captures[0].capturedKind == PieceType::King);
    CHECK(listener.captures[0].capturedBy == PieceColor::White);
    CHECK(game.isGameOver());
}

TEST_CASE("wait notifies onGameStarted exactly once on the first call") {
    GameEngine game = makeGame("Board:\nwR . .\n");
    RecordingListener listener;
    game.addListener(listener);

    game.wait(100);

    REQUIRE(listener.gameStarted.size() == 1);
}

TEST_CASE("onGameStarted is not fired again on subsequent wait calls") {
    GameEngine game = makeGame("Board:\nwR . .\n");
    RecordingListener listener;
    game.addListener(listener);

    game.wait(100);
    game.wait(100);
    game.wait(100);

    CHECK(listener.gameStarted.size() == 1);
}

TEST_CASE("a listener added after the game has already started does not receive onGameStarted retroactively") {
    GameEngine game = makeGame("Board:\nwR . .\n");
    game.wait(100);

    RecordingListener lateListener;
    game.addListener(lateListener);
    game.wait(100);

    CHECK(lateListener.gameStarted.empty());
}

TEST_CASE("capturing a king notifies onGameEnded with the correct winner exactly once") {
    GameEngine game = makeGame("Board:\nwR bK\n");
    RecordingListener listener;
    game.addListener(listener);

    game.requestMove(Position{0, 0}, Position{0, 1});
    game.wait(1000);

    REQUIRE(listener.gameEnded.size() == 1);
    CHECK(listener.gameEnded[0].winner == PieceColor::White);

    game.wait(1000);
    CHECK(listener.gameEnded.size() == 1);
}
