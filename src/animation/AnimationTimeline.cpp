#include "animation/AnimationTimeline.h"
#include <algorithm>

AnimationTimeline::AnimationTimeline(int frameCount, int framesPerSec, bool isLoop)
    : frameCount_(frameCount), framesPerSec_(framesPerSec), isLoop_(isLoop) {
}

int AnimationTimeline::frameIndexAt(long long elapsedMs) const {
    long long frameNumber = elapsedMs * framesPerSec_ / 1000;
    if (isLoop_) {
        return static_cast<int>(frameNumber % frameCount_);
    }
    return static_cast<int>(std::min<long long>(frameNumber, frameCount_ - 1));
}

bool AnimationTimeline::hasFinishedAt(long long elapsedMs) const {
    if (isLoop_) {
        return false;
    }
    long long frameNumber = elapsedMs * framesPerSec_ / 1000;
    return frameNumber >= frameCount_;
}
