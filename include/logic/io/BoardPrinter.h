#pragma once
#include <ostream>
#include <string>
#include "logic/engine/GameSnapshot.h"

void printBoard(const GameSnapshot& snapshot, std::ostream& out);
void printParseError(const std::string& errorMessage, std::ostream& out);
