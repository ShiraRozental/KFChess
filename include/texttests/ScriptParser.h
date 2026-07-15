#pragma once
#include <optional>
#include <string>
#include "texttests/ScriptCommand.h"

std::optional<Command> parseCommand(const std::string& line);
