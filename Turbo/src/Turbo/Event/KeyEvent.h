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
        KeyPressedEvent(const KeyCode code, u16 repeatCount)
            : KeyEvent(code), m_RepeatCount(repeatCount)
        {
        }

        u16 GetRepeatCount() const { return m_RepeatCount; }

        EVENT_CLASS_TYPE(KeyPressed)
    private:
        u16 m_RepeatCount;
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
