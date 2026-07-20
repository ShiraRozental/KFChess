#pragma once
#include "client/input/Controller.h"
#include "client/input/MouseEvent.h"

// The single place that decides what each mouse button means: left drives
// the Controller's select/move click flow, right issues a jump. Knows
// nothing about OpenCV or windows — it only consumes MouseEvent values.
class MouseInputRouter {
public:
    explicit MouseInputRouter(Controller& controller);

    void handle(MouseButton button, int pixelX, int pixelY);

private:
    Controller& controller_;
};
