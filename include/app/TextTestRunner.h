#pragma once
#include <optional>
#include <ostream>
#include <string>
#include "app/Controller.h"
#include "engine/GameEngine.h"

// Interprets the text DSL ("click x y" / "jump x y" / "wait ms" /
// "print board") used by the CLI entry point and by integration tests.
// Owns the one place in the codebase that still knows this text protocol
// speaks pixels — everything below it (GameEngine and down) works only in
// board cells.
class TextTestRunner {
public:
    explicit TextTestRunner(GameEngine& engine);

    void executeLine(const std::string& line, std::ostream& out);

private:
    void ensureController();

    GameEngine& engine_;
    std::optional<Controller> controller_;
};
