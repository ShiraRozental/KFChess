#pragma once
#include <variant>

struct ClickCommand { int x; int y; };
struct JumpCommand { int x; int y; };
struct WaitCommand { int ms; };
struct PrintBoardCommand {};

using Command = std::variant<ClickCommand, JumpCommand, WaitCommand, PrintBoardCommand>;
