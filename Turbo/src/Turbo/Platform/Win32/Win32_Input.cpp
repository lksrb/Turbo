#include "tbopch.h" 

#include "Turbo/Core/Input.h"

#include <windows.h>

namespace Turbo 
{
    bool Input::IsKeyPressed(KeyCode keyCode)
    {
        return (::GetAsyncKeyState(static_cast<int>(keyCode)) & 0x8000) != 0;
    }

    bool Input::IsKeyReleased(KeyCode keyCode)
    {
        return (::GetAsyncKeyState(static_cast<int>(keyCode)) & 0x8000) == 0;
    }
}
