#include "doctest/doctest.h"
#include "animation/AnimationConfig.h"
#include <filesystem>

namespace {
    std::filesystem::path fixture(const std::string& fileName) {
        return std::filesystem::path(TEST_FIXTURES_DIR) / "animation" / fileName;
    }
}

TEST_CASE("a valid config.json loads every field correctly") {
    AnimationConfig config = loadAnimationConfig(fixture("valid_config.json"));

    CHECK(config.speedMPerSec == doctest::Approx(1.5));
    CHECK(config.nextStateWhenFinished == "long_rest");
    CHECK(config.framesPerSec == 12);
    CHECK(config.isLoop == true);
}

TEST_CASE("a missing config file is a clear rejection, not a silent default") {
    CHECK_THROWS_AS(loadAnimationConfig(fixture("does_not_exist.json")), std::runtime_error);
}

TEST_CASE("malformed JSON is a clear rejection, not a silent default") {
    CHECK_THROWS_AS(loadAnimationConfig(fixture("malformed_config.json")), std::runtime_error);
}

TEST_CASE("a config missing a required field is a clear rejection, not a silent default") {
    CHECK_THROWS_AS(loadAnimationConfig(fixture("missing_field_config.json")), std::runtime_error);
}

TEST_CASE("a non-positive frames_per_sec is a clear rejection, not a silent default") {
    CHECK_THROWS_AS(loadAnimationConfig(fixture("zero_fps_config.json")), std::runtime_error);
}
