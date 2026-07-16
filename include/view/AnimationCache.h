#pragma once
#include <filesystem>
#include <map>
#include <string>
#include "model/PieceColor.h"
#include "model/PieceType.h"
#include "view/SpriteAnimation.h"

class AnimationCache {
public:
    AnimationCache(std::filesystem::path assetsRoot, int cellSizePixels);

    const std::map<std::string, SpriteAnimation>& animationsFor(PieceColor color, PieceType kind);

private:
    std::filesystem::path assetsRoot_;
    int cellSizePixels_;
    std::map<std::string, std::map<std::string, SpriteAnimation>> loadedByCode_;
};
