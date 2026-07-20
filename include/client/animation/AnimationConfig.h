#pragma once
#include <filesystem>
#include <string>

// Data-driven contents of one state folder's config.json. next_state_when_finished
// is deliberately kept as plain data here (not a class hierarchy) — the
// lookup itself is the state transition table (see PLAN.md's Design patterns map).
struct AnimationConfig {
    double speedMPerSec;
    std::string nextStateWhenFinished;
    int framesPerSec;
    bool isLoop;
};

// Throws std::runtime_error if the file is missing, is not valid JSON, is
// missing a required field, or has a non-positive frames_per_sec — a
// malformed config is a clear rejection, never a silent default.
AnimationConfig loadAnimationConfig(const std::filesystem::path& configJsonPath);
