#include <sstream>
#include <string>
#include <thread>
#include "engine/GameEngine.h"
#include "input/BoardMapper.h"
#include "io/BoardParser.h"
#include "view/ImageView.h"

namespace {
    constexpr int kCellSizePixels = 100;
    constexpr int kFrameMs = 33;
    constexpr int kTotalFrames = 300;
    constexpr int kPawnMoveFrame = 30;
    constexpr int kKnightMoveFrame = 60;
    constexpr int kQueenJumpFrame = 90;
    constexpr const char* kAssetsRoot = "assets/images/pieces";
    constexpr const char* kBoardImage = "assets/images/board.png";

    const std::string kStandardBoard =
        "Board:\n"
        "bR bN bB bQ bK bB bN bR\n"
        "bP bP bP bP bP bP bP bP\n"
        ". . . . . . . .\n"
        ". . . . . . . .\n"
        ". . . . . . . .\n"
        ". . . . . . . .\n"
        "wP wP wP wP wP wP wP wP\n"
        "wR wN wB wQ wK wB wN wR\n";
}

int main() {
    std::istringstream boardText(kStandardBoard);
    ParsedInput parsed;
    std::string errorMessage;
    if (!BoardParser::parse(boardText, parsed, errorMessage)) {
        return 1;
    }

    int rows = parsed.board.rowCount();
    int cols = parsed.board.colCount();
    GameEngine game(std::move(parsed.board));
    ImageView view(BoardMapper(rows, cols, kCellSizePixels), kAssetsRoot, kBoardImage);

    for (int frame = 0; frame < kTotalFrames; ++frame) {
        if (frame == kPawnMoveFrame) game.requestMove(Position{6, 4}, Position{4, 4});
        if (frame == kKnightMoveFrame) game.requestMove(Position{0, 1}, Position{2, 2});
        if (frame == kQueenJumpFrame) game.requestJump(Position{7, 3});

        game.wait(kFrameMs);
        view.render(game.snapshot());
        std::this_thread::sleep_for(std::chrono::milliseconds(kFrameMs));
    }
    return 0;
}
