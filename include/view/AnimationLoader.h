#pragma once
#include <filesystem>
#include <map>
#include <string>
#include "model/PieceColor.h"
#include "model/PieceType.h"
#include "view/SpriteAnimation.h"

std::map<std::string, SpriteAnimation> loadPieceAnimations(
    PieceColor color, PieceType kind, const std::filesystem::path& assetsRoot);
