#pragma once
#include <chrono>
#include <filesystem>
#include <map>
#include <optional>
#include <set>
#include "logic/engine/MovesLog.h"
#include "logic/engine/ScoreBoard.h"
#include "client/input/BoardMapper.h"
#include "client/view/AnimationCache.h"
#include "client/view/GameScreenLayout.h"
#include "client/view/Img.h"
#include "client/view/PieceView.h"
#include "client/view/PlayerNames.h"
#include "client/view/Renderer.h"

class ImageView : public Renderer {
public:
    ImageView(BoardMapper mapper, GameScreenLayout layout,
              const MovesLog& movesLog, const ScoreBoard& scoreBoard,
              PlayerNames playerNames, const std::filesystem::path& assetsRoot,
              const std::filesystem::path& boardImagePath);

    void render(const GameSnapshot& snapshot) override;
    void setSelectedCell(std::optional<Position> cell);
    void setLegalDestinations(std::set<Position> destinations);

private:
    struct TrackedPiece {
        PieceType kind;
        PieceView view;
    };

    long long elapsedSinceLastRenderMs();
    PieceView& viewFor(const Piece& piece);
    Img buildBackground(const std::filesystem::path& boardImagePath);
    void drawBoardLabels(Img& canvas);
    void drawCenteredText(Img& target, const std::string& text, int centerX,
                          int baselineY, double fontScale, int thickness);
    void drawMovesPanel(Img& frame, PieceColor color);
    void drawPlayerBanner(Img& frame, PieceColor color);
    void drawGameOverOverlay(Img& frame, std::optional<PieceColor> winner);
    void drawSelectionHighlight(Img& frame);
    void drawLegalDestinationMarkers(Img& frame);
    void drawPieces(const GameSnapshot& snapshot, Img& frame, long long dtMs);
    void drawCooldownBar(Img& frame, PixelPoint cellTopLeft, double progress);
    void removeViewsOfCapturedPieces(const Board& board);

    BoardMapper mapper_;
    GameScreenLayout layout_;
    const MovesLog& movesLog_;
    const ScoreBoard& scoreBoard_;
    PlayerNames playerNames_;
    int pieceMarginPixels_;
    AnimationCache cache_;
    Img background_;
    std::map<PieceId, TrackedPiece> pieceViews_;
    std::optional<std::chrono::steady_clock::time_point> lastRenderTime_;
    std::optional<Position> selectedCell_;
    std::set<Position> legalDestinations_;
};
