#pragma once
#include <ostream>
#include <string>
#include "engine/GameEngine.h"
#include "input/Controller.h"
#include "texttests/ScriptCommand.h"

class ScriptRunner {
public:
    ScriptRunner(Controller& controller, GameEngine& engine, std::ostream& out);

    void run(const Command& command);

private:
    Controller& controller_;
    GameEngine& engine_;
    std::ostream& out_;
};

void runLine(ScriptRunner& runner, const std::string& line);
