#pragma once
#include <filesystem>
#include <map>
#include <string>
#include "logic/model/PieceColor.h"
#include "logic/model/PieceType.h"
#include "client/view/SpriteAnimation.h"

std::map<std::string, SpriteAnimation> loadPieceAnimations(
    PieceColor color, PieceType kind, const std::filesystem::path& assetsRoot,
    const std::pair<int, int>& frameSize = {});
