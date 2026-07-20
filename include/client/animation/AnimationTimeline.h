#pragma once

class AnimationTimeline {
public:
    AnimationTimeline(int frameCount, int framesPerSec, bool isLoop);

    int frameIndexAt(long long elapsedMs) const;
    bool hasFinishedAt(long long elapsedMs) const;

private:
    int frameCount_;
    int framesPerSec_;
    bool isLoop_;
};
