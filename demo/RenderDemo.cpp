#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include "engine/GameEngine.h"
#include "input/BoardMapper.h"
#include "io/BoardParser.h"
#include "view/ImageView.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace {
    constexpr int kCellSizePixels = 100;
    constexpr int kFrameMs = 33;
    constexpr int kTotalFrames = 300;
    constexpr int kPawnMoveFrame = 30;
    constexpr int kKnightMoveFrame = 60;
    constexpr int kQueenJumpFrame = 90;
    constexpr const char* kAssetsDirName = "assets";

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

    // On Windows argv[0] is ANSI-encoded, which corrupts non-Latin path
    // characters (e.g. a Hebrew user folder), so the executable path is
    // taken from the wide-character API instead.
    std::filesystem::path executablePath(const char* argv0) {
#ifdef _WIN32
        wchar_t buffer[MAX_PATH];
        DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
        if (length > 0 && length < MAX_PATH) {
            return std::filesystem::path(std::wstring(buffer, length));
        }
#endif
        return std::filesystem::path(argv0);
    }

    // Walks up from the executable's directory until it finds the project
    // root (the directory containing assets/), so the demo works no matter
    // which directory it is launched from.
    std::filesystem::path findAssetsRoot(const std::filesystem::path& exePath) {
        for (auto dir = exePath.parent_path(); !dir.empty(); dir = dir.parent_path()) {
            if (std::filesystem::exists(dir / kAssetsDirName)) return dir / kAssetsDirName;
            if (dir == dir.parent_path()) break;
        }
        throw std::runtime_error("Cannot find the assets/ directory above " + exePath.string());
    }

    int runDemo(const std::filesystem::path& exePath) {
        std::istringstream boardText(kStandardBoard);
        ParsedInput parsed;
        std::string errorMessage;
        if (!BoardParser::parse(boardText, parsed, errorMessage)) {
            throw std::runtime_error("Board parse failed: " + errorMessage);
        }

        std::filesystem::path assets = findAssetsRoot(exePath);
        int rows = parsed.board.rowCount();
        int cols = parsed.board.colCount();
        GameEngine game(std::move(parsed.board));
        ImageView view(BoardMapper(rows, cols, kCellSizePixels),
                       assets / "images" / "pieces",
                       assets / "images" / "board_classic.png");

        for (int frame = 0; frame < kTotalFrames; ++frame) {
            if (frame == kPawnMoveFrame) game.requestMove(Position{6, 4}, Position{4, 4});
            if (frame == kKnightMoveFrame) game.requestMove(Position{0, 1}, Position{2, 2});
            if (frame == kQueenJumpFrame) game.requestJump(Position{7, 3});

            game.wait(kFrameMs);
            view.render(game.snapshot());
            std::this_thread::sleep_for(std::chrono::milliseconds(kFrameMs));
        }

        std::cout << "Demo finished - press any key in the image window to close" << std::endl;
        Img::wait_for_key();
        return 0;
    }
}

int main(int argc, char* argv[]) {
    try {
        return runDemo(executablePath(argv[0]));
    } catch (const std::exception& error) {
        std::cerr << "RenderDemo failed: " << error.what() << std::endl;
        return 1;
    }
}
