#pragma once

#include "Turbo/Core/KeyCodes.h"
#include "Turbo/Core/MouseCodes.h"

namespace Turbo {

    namespace Input
    {
        bool IsKeyPressed(KeyCode keyCode);
        bool IsKeyReleased(KeyCode keyCode);
    };
}
