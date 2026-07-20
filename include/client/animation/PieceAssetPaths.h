#pragma once
#include <filesystem>
#include <map>
#include <string>
#include "logic/model/PieceColor.h"
#include "logic/model/PieceType.h"

std::string pieceAssetCode(PieceColor color, PieceType kind);

std::map<std::string, std::filesystem::path> resolveStateFolders(
    PieceColor color, PieceType kind, const std::filesystem::path& assetsRoot);
