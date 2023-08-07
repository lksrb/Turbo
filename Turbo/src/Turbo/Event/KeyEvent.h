#pragma once

#include "Turbo/Core/KeyCodes.h"
#include "Turbo/Event/Event.h"

namespace Turbo 
{
    class KeyEvent : public Event 
    {
    public:
        KeyCode GetKeyCode() const { return m_KeyCode; }

        EVENT_CLASS_CATEGORY(EventCategory_Keyboard | EventCategory_Input)
    protected:
        KeyEvent(const KeyCode keycode)
            : m_KeyCode(keycode)
        {
        }
    private:
        KeyCode m_KeyCode;
    };

    class KeyPressedEvent : public KeyEvent 
    {
    public:
        KeyPressedEvent(const KeyCode code, bool repeat)
            : KeyEvent(code), m_IsRepeated(repeat)
        {
        }

        bool IsRepeated() const { return m_IsRepeated; }

        EVENT_CLASS_TYPE(KeyPressed)
    private:
        bool m_IsRepeated;
    };

    class KeyReleasedEvent : public KeyEvent 
    {
    public:
        KeyReleasedEvent(const KeyCode keycode)
            : KeyEvent(keycode)
        {
        }

        EVENT_CLASS_TYPE(KeyReleased)

    };

    class KeyTypedEvent : public KeyEvent 
    {
    public:
        KeyTypedEvent(const KeyCode keycode)
            : KeyEvent(keycode)
        {
        }

        EVENT_CLASS_TYPE(KeyTyped)
    };
}
