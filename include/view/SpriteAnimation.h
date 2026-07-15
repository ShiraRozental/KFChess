#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include "animation/AnimationConfig.h"
#include "animation/AnimationTimeline.h"
#include "view/Img.h"

// Owns one state's loaded sprite frames (needs Img/OpenCV, so this class -
// unlike AnimationTimeline/AnimationConfig - is excluded from KFChessTests;
// see PLAN.md Step 2). All frame-index/looping/finish math is delegated to
// AnimationTimeline, so a single SpriteAnimation instance can be shared
// across every piece currently in this (piece code, state) combination.
class SpriteAnimation {
public:
    explicit SpriteAnimation(const std::filesystem::path& stateFolder);

    const Img& frameAt(long long elapsedMs) const;
    bool hasFinishedAt(long long elapsedMs) const;
    const std::string& nextStateWhenFinished() const;

private:
    SpriteAnimation(std::vector<Img> frames, AnimationConfig config);

    std::vector<Img> frames_;
    AnimationTimeline timeline_;
    std::string nextStateWhenFinished_;
};
