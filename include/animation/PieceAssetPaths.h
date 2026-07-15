#pragma once
#include <filesystem>
#include <map>
#include <string>
#include "model/PieceColor.h"
#include "model/PieceType.h"

std::string pieceAssetCode(PieceColor color, PieceType kind);

std::map<std::string, std::filesystem::path> resolveStateFolders(
    PieceColor color, PieceType kind, const std::filesystem::path& assetsRoot);
