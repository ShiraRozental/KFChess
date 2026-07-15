#include "texttests/ScriptRunner.h"
#include "texttests/ScriptParser.h"
#include "io/BoardPrinter.h"

namespace {
    struct Dispatch {
        Controller& controller;
        GameEngine& engine;
        std::ostream& out;

        void operator()(const ClickCommand& cmd) { controller.click(cmd.x, cmd.y); }
        void operator()(const JumpCommand& cmd) { controller.jump(cmd.x, cmd.y); }
        void operator()(const WaitCommand& cmd) { engine.wait(cmd.ms); }
        void operator()(const PrintBoardCommand&) {
            printBoard(engine.snapshot(), out);
            out << "\n";
        }
    };
}

ScriptRunner::ScriptRunner(Controller& controller, GameEngine& engine, std::ostream& out)
    : controller_(controller), engine_(engine), out_(out) {
}

void ScriptRunner::run(const Command& command) {
    std::visit(Dispatch{controller_, engine_, out_}, command);
}

void runLine(ScriptRunner& runner, const std::string& line) {
    if (std::optional<Command> command = parseCommand(line)) {
        runner.run(*command);
    }
}
