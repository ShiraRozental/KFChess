#include "client/animation/PieceAssetPaths.h"
#include "client/animation/AnimationStateNames.h"
#include "logic/io/PieceNotation.h"
#include <array>
#include <cctype>
#include <stdexcept>

namespace {
    const std::array<std::string, 5> kRequiredStates = {
        AnimationStateName::Idle, AnimationStateName::Move, AnimationStateName::Jump,
        AnimationStateName::ShortRest, AnimationStateName::LongRest};
}

std::string pieceAssetCode(PieceColor color, PieceType kind) {
    char colorChar = static_cast<char>(std::toupper(static_cast<unsigned char>(pieceColorToChar(color))));
    return std::string(1, pieceTypeToChar(kind)) + std::string(1, colorChar);
}

std::map<std::string, std::filesystem::path> resolveStateFolders(
    PieceColor color, PieceType kind, const std::filesystem::path& assetsRoot) {
    std::string code = pieceAssetCode(color, kind);
    std::filesystem::path statesFolder = assetsRoot / code / "states";

    std::map<std::string, std::filesystem::path> stateFolders;
    for (const auto& state : kRequiredStates) {
        std::filesystem::path stateFolder = statesFolder / state;
        if (!std::filesystem::is_directory(stateFolder)) {
            throw std::runtime_error("Missing animation state \"" + state + "\" for piece " + code +
                                      ": " + stateFolder.string());
        }
        stateFolders[state] = stateFolder;
    }
    return stateFolders;
}
