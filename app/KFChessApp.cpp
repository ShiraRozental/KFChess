#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include "engine/GameEngine.h"
#include "engine/MovesLog.h"
#include "engine/ScoreBoard.h"
#include "input/BoardMapper.h"
#include "input/Controller.h"
#include "input/MouseInputRouter.h"
#include "io/BoardParser.h"
#include "view/GameScreenLayout.h"
#include "view/ImageView.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace {
    constexpr int kCellSizePixels = 100;
    constexpr int kFrameMs = 33;
    constexpr int kEscapeKeyCode = 27;
    constexpr const char* kAssetsDirName = "assets";
    constexpr const char* kWhitePlayerName = "White";
    constexpr const char* kBlackPlayerName = "Black";

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
    // root (the directory containing assets/), so the game works no matter
    // which directory it is launched from.
    std::filesystem::path findAssetsRoot(const std::filesystem::path& exePath) {
        for (auto dir = exePath.parent_path(); !dir.empty(); dir = dir.parent_path()) {
            if (std::filesystem::exists(dir / kAssetsDirName)) return dir / kAssetsDirName;
            if (dir == dir.parent_path()) break;
        }
        throw std::runtime_error("Cannot find the assets/ directory above " + exePath.string());
    }

    const char* winnerName(std::optional<PieceColor> winner) {
        if (!winner.has_value()) return "Nobody";
        return *winner == PieceColor::White ? "White" : "Black";
    }

    int runGame(const std::filesystem::path& exePath) {
        std::istringstream boardText(kStandardBoard);
        ParsedInput parsed;
        std::string errorMessage;
        if (!BoardParser::parse(boardText, parsed, errorMessage)) {
            throw std::runtime_error("Board parse failed: " + errorMessage);
        }

        std::filesystem::path assets = findAssetsRoot(exePath);
        int rows = parsed.board.rowCount();
        int cols = parsed.board.colCount();
        GameScreenLayout layout(rows, cols, kCellSizePixels);
        BoardMapper mapper(rows, cols, kCellSizePixels, layout.boardOrigin());
        GameEngine game(std::move(parsed.board));
        MovesLog movesLog(rows);
        ScoreBoard scoreBoard;
        game.addListener(movesLog);
        game.addListener(scoreBoard);
        ImageView view(mapper, layout, movesLog, scoreBoard,
                       PlayerNames{kWhitePlayerName, kBlackPlayerName},
                       assets / "images" / "pieces",
                       assets / "images" / "board_classic.png");
        Controller controller(game, mapper);
        MouseInputRouter router(controller);

        Img::set_mouse_handler([&](MouseButton button, int pixelX, int pixelY) {
            router.handle(button, pixelX, pixelY);
            std::optional<Position> selected = controller.selectedCell();
            view.setSelectedCell(selected);
            view.setLegalDestinations(selected ? game.legalDestinationsFrom(*selected)
                                               : std::set<Position>{});
        });

        std::cout << "Left click: select piece, then destination. "
                     "Right click: jump. ESC: quit." << std::endl;

        while (true) {
            game.wait(kFrameMs);
            GameSnapshot snapshot = game.snapshot();
            view.render(snapshot);

            if (snapshot.isGameOver()) {
                std::cout << winnerName(snapshot.winner()) << " wins!" << std::endl;
                std::cout << "Press any key in the image window to close" << std::endl;
                Img::wait_for_key();
                break;
            }
            if (Img::wait_key(kFrameMs) == kEscapeKeyCode) break;
        }
        return 0;
    }
}

int main(int argc, char* argv[]) {
    try {
        return runGame(executablePath(argv[0]));
    } catch (const std::exception& error) {
        std::cerr << "KFChess failed: " << error.what() << std::endl;
        return 1;
    }
}
