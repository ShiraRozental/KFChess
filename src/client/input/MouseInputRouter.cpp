#include "client/input/MouseInputRouter.h"

MouseInputRouter::MouseInputRouter(Controller& controller)
    : controller_(controller) {
}

void MouseInputRouter::handle(MouseButton button, int pixelX, int pixelY) {
    switch (button) {
        case MouseButton::Left:
            controller_.click(pixelX, pixelY);
            break;
        case MouseButton::Right:
            controller_.jump(pixelX, pixelY);
            break;
    }
}
