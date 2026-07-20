#include "logic/io/TimestampFormat.h"
#include <cstdio>

namespace {
    constexpr long long kMillisPerSecond = 1000;
    constexpr long long kSecondsPerMinute = 60;
    constexpr int kFormattedSizeLimit = 32;
}

std::string formatClockMs(long long ms) {
    if (ms < 0) ms = 0;
    long long minutes = ms / (kMillisPerSecond * kSecondsPerMinute);
    long long seconds = (ms / kMillisPerSecond) % kSecondsPerMinute;
    long long millis = ms % kMillisPerSecond;

    char formatted[kFormattedSizeLimit];
    std::snprintf(formatted, sizeof(formatted), "%02lld:%02lld.%03lld",
                  minutes, seconds, millis);
    return formatted;
}
