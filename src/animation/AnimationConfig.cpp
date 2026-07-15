#include "animation/AnimationConfig.h"
#include <fstream>
#include <stdexcept>
#include "nlohmann/json.hpp"

namespace {
    const nlohmann::json& requireField(const nlohmann::json& parent, const std::string& key,
                                        const std::filesystem::path& configJsonPath) {
        if (!parent.contains(key)) {
            throw std::runtime_error("Animation config missing field \"" + key + "\": " + configJsonPath.string());
        }
        return parent.at(key);
    }
}

AnimationConfig loadAnimationConfig(const std::filesystem::path& configJsonPath) {
    std::ifstream file(configJsonPath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open animation config: " + configJsonPath.string());
    }

    nlohmann::json root;
    try {
        file >> root;
    } catch (const nlohmann::json::parse_error& error) {
        throw std::runtime_error("Malformed animation config " + configJsonPath.string() + ": " + error.what());
    }

    const auto& physics = requireField(root, "physics", configJsonPath);
    const auto& graphics = requireField(root, "graphics", configJsonPath);

    AnimationConfig config;
    config.speedMPerSec = requireField(physics, "speed_m_per_sec", configJsonPath).get<double>();
    config.nextStateWhenFinished = requireField(physics, "next_state_when_finished", configJsonPath).get<std::string>();
    config.framesPerSec = requireField(graphics, "frames_per_sec", configJsonPath).get<int>();
    config.isLoop = requireField(graphics, "is_loop", configJsonPath).get<bool>();

    if (config.framesPerSec <= 0) {
        throw std::runtime_error("Animation config has non-positive frames_per_sec: " + configJsonPath.string());
    }

    return config;
}
