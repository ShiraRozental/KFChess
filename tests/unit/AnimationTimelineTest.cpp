#include "doctest/doctest.h"
#include "animation/AnimationTimeline.h"

TEST_CASE("frameIndexAt is frame 0 right at the start, whether looping or not") {
    AnimationTimeline looping(5, 10, true);
    AnimationTimeline nonLooping(5, 10, false);
    CHECK(looping.frameIndexAt(0) == 0);
    CHECK(nonLooping.frameIndexAt(0) == 0);
}

TEST_CASE("frameIndexAt advances exactly at each frame's time boundary, at 10fps") {
    AnimationTimeline timeline(5, 10, false); // 100ms per frame

    CHECK(timeline.frameIndexAt(99) == 0);
    CHECK(timeline.frameIndexAt(100) == 1);
    CHECK(timeline.frameIndexAt(199) == 1);
    CHECK(timeline.frameIndexAt(200) == 2);
    CHECK(timeline.frameIndexAt(399) == 3);
    CHECK(timeline.frameIndexAt(400) == 4);
}

TEST_CASE("a looping timeline wraps back to frame 0 after a full cycle and keeps counting") {
    AnimationTimeline timeline(5, 10, true); // 500ms per full cycle

    CHECK(timeline.frameIndexAt(499) == 4);
    CHECK(timeline.frameIndexAt(500) == 0);
    CHECK(timeline.frameIndexAt(600) == 1);
    CHECK(timeline.frameIndexAt(999) == 4);
    CHECK(timeline.frameIndexAt(1000) == 0);
}

TEST_CASE("a non-looping timeline holds on the last frame once its total duration has passed") {
    AnimationTimeline timeline(5, 10, false);

    CHECK(timeline.frameIndexAt(500) == 4);
    CHECK(timeline.frameIndexAt(10000) == 4);
}

TEST_CASE("hasFinishedAt becomes true exactly when the frame after the last one would start") {
    AnimationTimeline timeline(5, 10, false);

    CHECK_FALSE(timeline.hasFinishedAt(499));
    CHECK(timeline.hasFinishedAt(500));
}

TEST_CASE("hasFinishedAt is always false for a looping timeline, however much time elapses") {
    AnimationTimeline timeline(5, 10, true);

    CHECK_FALSE(timeline.hasFinishedAt(0));
    CHECK_FALSE(timeline.hasFinishedAt(500));
    CHECK_FALSE(timeline.hasFinishedAt(1000000));
}
