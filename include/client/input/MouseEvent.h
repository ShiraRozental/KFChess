#pragma once
#include <functional>

// The OpenCV-free contract between the window layer (which translates raw
// OpenCV mouse events) and the input layer (which consumes them). Keeping
// it here lets MouseInputRouter build and test without any OpenCV headers.
enum class MouseButton { Left, Right };

using MouseHandler = std::function<void(MouseButton button, int pixelX, int pixelY)>;
