#pragma once
#include <ostream>
#include <string>
#include "logic/engine/IGameEngine.h"
#include "client/input/Controller.h"
#include "texttests/ScriptCommand.h"

class ScriptRunner {
public:
    ScriptRunner(Controller& controller, IGameEngine& engine, std::ostream& out);

    void run(const Command& command);

private:
    Controller& controller_;
    IGameEngine& engine_;
    std::ostream& out_;
};

void runLine(ScriptRunner& runner, const std::string& line);
