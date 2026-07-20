#include "client/view/AnimationCache.h"
#include "client/animation/PieceAssetPaths.h"
#include "client/view/AnimationLoader.h"

AnimationCache::AnimationCache(std::filesystem::path assetsRoot, int cellSizePixels)
    : assetsRoot_(std::move(assetsRoot)), cellSizePixels_(cellSizePixels) {
}

const std::map<std::string, SpriteAnimation>& AnimationCache::animationsFor(
    PieceColor color, PieceType kind) {
    std::string code = pieceAssetCode(color, kind);
    auto found = loadedByCode_.find(code);
    if (found == loadedByCode_.end()) {
        found = loadedByCode_.emplace(
            code, loadPieceAnimations(color, kind, assetsRoot_,
                                      {cellSizePixels_, cellSizePixels_})).first;
    }
    return found->second;
}
