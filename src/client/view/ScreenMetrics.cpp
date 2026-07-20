#include "client/view/ScreenMetrics.h"
#include <algorithm>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

namespace {
    constexpr int kFallbackWidth = 1024;
    constexpr int kFallbackHeight = 700;

    PixelSize desktopWorkArea() {
#ifdef _WIN32
        RECT workArea;
        if (SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0)) {
            return PixelSize{workArea.right - workArea.left,
                             workArea.bottom - workArea.top};
        }
#endif
        return PixelSize{kFallbackWidth, kFallbackHeight};
    }

    PixelSize windowChromeSize() {
#ifdef _WIN32
        return PixelSize{2 * GetSystemMetrics(SM_CXSIZEFRAME),
                         2 * GetSystemMetrics(SM_CYSIZEFRAME) + GetSystemMetrics(SM_CYCAPTION)};
#else
        return PixelSize{0, 0};
#endif
    }
}

PixelSize ScreenMetrics::availableWindowSize() {
    const PixelSize workArea = desktopWorkArea();
    const PixelSize chrome = windowChromeSize();
    return PixelSize{std::max(1, workArea.width - chrome.width),
                     std::max(1, workArea.height - chrome.height)};
}
