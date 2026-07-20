#include "client/view/AnimationLoader.h"
#include "client/animation/PieceAssetPaths.h"

std::map<std::string, SpriteAnimation> loadPieceAnimations(
    PieceColor color, PieceType kind, const std::filesystem::path& assetsRoot,
    const std::pair<int, int>& frameSize) {
    std::map<std::string, SpriteAnimation> animations;
    for (const auto& [state, folder] : resolveStateFolders(color, kind, assetsRoot)) {
        animations.emplace(state, SpriteAnimation(folder, frameSize));
    }
    return animations;
}
