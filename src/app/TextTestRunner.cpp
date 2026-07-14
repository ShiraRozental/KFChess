#include "app/TextTestRunner.h"
#include "io/BoardMapper.h"
#include "io/BoardTextFormat.h"
#include <sstream>

namespace {
    constexpr int kCellSizePixels = 100;
}

TextTestRunner::TextTestRunner(GameEngine& engine) : engine_(engine) {
}

// Board dimensions aren't known until after loadBoard, so the Controller
// (and the BoardMapper it owns) is built lazily on first use rather than
// in the constructor.
void TextTestRunner::ensureController() {
    if (controller_.has_value()) return;
    const Board& board = engine_.snapshot().board();
    BoardMapper mapper(board.rowCount(), board.colCount(), kCellSizePixels);
    controller_.emplace(engine_, mapper);
}

// Executes a single command line based on its keyword.
void TextTestRunner::executeLine(const std::string& line, std::ostream& out) {
    std::istringstream ss(line);
    std::string keyword;
    ss >> keyword;

    if (keyword == "click") {
        int x, y;
        if (ss >> x >> y) {
            ensureController();
            controller_->click(x, y);
        }
    } else if (keyword == "jump") {
        int x, y;
        if (ss >> x >> y) {
            ensureController();
            controller_->jump(x, y);
        }
    } else if (keyword == "wait") {
        int ms;
        if (ss >> ms) engine_.wait(ms);
    } else if (keyword == "print") {
        std::string second;
        ss >> second;
        if (second == "board") {
            BoardTextFormat::write(engine_.snapshot().board(), out);
            out << "\n";
        }
    }
    // unknown keyword: ignored
}
