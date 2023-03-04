#pragma once

#include "Turbo/core/MouseCodes.h"
#include "Turbo/event/event.h"

namespace Turbo
{

    class MouseMovedEvent : public Event
    {
    public:
        MouseMovedEvent(f32 x, f32 y)
            : m_MouseX(x), m_MouseY(y)
        {
        }

        f32 GetX() const { return m_MouseX; }
        f32 GetY() const { return m_MouseY; }

        EVENT_CLASS_TYPE(MouseMoved)
            EVENT_CLASS_CATEGORY(EventCategory_Mouse | EventCategory_Input)
    private:
        f32 m_MouseX, m_MouseY;
    };

    class MouseScrolledEvent : public Event
    {
    private:
        f32 m_XOffset, m_YOffset;

    public:
        MouseScrolledEvent(const f32 xOffset, const f32 yOffset)
            : m_XOffset(xOffset), m_YOffset(yOffset)
        {
        }

        f32 GetOffsetX() const { return m_XOffset; }
        f32 GetOffsetY() const { return m_YOffset; }

        EVENT_CLASS_TYPE(MouseScrolled)
            EVENT_CLASS_CATEGORY(EventCategory_Mouse | EventCategory_Input)

    };

    class MouseButtonEvent : public Event
    {
    public:
        MouseCode GetMouseButton() const { return m_Button; }

        EVENT_CLASS_CATEGORY(EventCategory_Mouse | EventCategory_Input | EventCategory_MouseButton)
    protected:
        MouseButtonEvent(MouseCode button)
            : m_Button(button)
        {
        }

        MouseCode m_Button;
    };

    class MouseButtonPressedEvent : public MouseButtonEvent
    {
    public:
        MouseButtonPressedEvent(MouseCode button, i32 x, i32 y)
            : MouseButtonEvent(button), m_MouseX(x), m_MouseY(y)
        {
        }

        i32 GetMouseX() const { return m_MouseX; }
        i32 GetMouseY() const { return m_MouseY; }

        EVENT_CLASS_TYPE(MouseButtonPressed)
    private:
        i32 m_MouseX, m_MouseY;
    };

    class MouseButtonReleasedEvent : public MouseButtonEvent
    {
    public:
        MouseButtonReleasedEvent(MouseCode button, i32 x, i32 y)
            : MouseButtonEvent(button), m_MouseX(x), m_MouseY(y)
        {
        }

        i32 GetMouseX() const { return m_MouseX; }
        i32 GetMouseY() const { return m_MouseY; }

        EVENT_CLASS_TYPE(MouseButtonReleased)
    private:
        i32 m_MouseX, m_MouseY;
    };

}
