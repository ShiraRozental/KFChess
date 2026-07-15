#include "view/SpriteAnimation.h"
#include <algorithm>
#include <stdexcept>

namespace {
    std::vector<Img> loadFrames(const std::filesystem::path& spritesFolder) {
        std::vector<std::filesystem::path> paths;
        for (const auto& entry : std::filesystem::directory_iterator(spritesFolder)) {
            paths.push_back(entry.path());
        }
        if (paths.empty()) {
            throw std::runtime_error("Animation has no sprite frames: " + spritesFolder.string());
        }

        std::sort(paths.begin(), paths.end(), [](const auto& a, const auto& b) {
            return std::stoi(a.stem().string()) < std::stoi(b.stem().string());
        });

        std::vector<Img> frames;
        frames.reserve(paths.size());
        for (const auto& path : paths) {
            frames.emplace_back(Img().read(path.string()));
        }
        return frames;
    }
}

SpriteAnimation::SpriteAnimation(const std::filesystem::path& stateFolder)
    : SpriteAnimation(loadFrames(stateFolder / "sprites"),
                       loadAnimationConfig(stateFolder / "config.json")) {
}

SpriteAnimation::SpriteAnimation(std::vector<Img> frames, AnimationConfig config)
    : frames_(std::move(frames)),
      timeline_(static_cast<int>(frames_.size()), config.framesPerSec, config.isLoop),
      nextStateWhenFinished_(std::move(config.nextStateWhenFinished)) {
}

const Img& SpriteAnimation::frameAt(long long elapsedMs) const {
    return frames_[timeline_.frameIndexAt(elapsedMs)];
}

bool SpriteAnimation::hasFinishedAt(long long elapsedMs) const {
    return timeline_.hasFinishedAt(elapsedMs);
}

const std::string& SpriteAnimation::nextStateWhenFinished() const {
    return nextStateWhenFinished_;
}
