#pragma once
#include "view/PixelSize.h"

// Screen geometry as reported by the host OS. Isolated so that the rest of
// the view layer stays platform independent.
namespace ScreenMetrics {
    // Usable desktop area, excluding permanently docked bars such as the
    // Windows taskbar, and excluding the window's own title bar and borders.
    PixelSize availableWindowSize();
}
