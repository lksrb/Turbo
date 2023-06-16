#pragma once

#include "Turbo/Event/Event.h"

namespace Turbo
{
    class WindowResizeEvent : public Event
    {
    public:
        WindowResizeEvent(u32 width, u32 height)
            : m_Width(width), m_Height(height)
        {
        }

        u32 GetWidth() const { return m_Width; }
        u32 GetHeight() const { return m_Height; }

        EVENT_CLASS_TYPE(WindowResize)
            EVENT_CLASS_CATEGORY(EventCategory_Application)
    private:
        u32 m_Width, m_Height;
    };

    class WindowFocusedEvent : public Event
    {
    public:
        WindowFocusedEvent() {}

        EVENT_CLASS_TYPE(WindowFocus)
            EVENT_CLASS_CATEGORY(EventCategory_Application)
    };

    class WindowLostFocusEvent : public Event
    {
    public:
        WindowLostFocusEvent() {}

        EVENT_CLASS_TYPE(WindowKillFocus)
            EVENT_CLASS_CATEGORY(EventCategory_Application)
    };

    class WindowMovedEvent : public Event
    {
    public:
        WindowMovedEvent(u32 x, u32 y)
            : m_X(x), m_Y(y)
        {
        }

        u32 GetWidth() const { return m_X; }
        u32 GetHeight() const { return m_Y; }

        EVENT_CLASS_TYPE(WindowMoved)
            EVENT_CLASS_CATEGORY(EventCategory_Application)

    private:
        u32 m_X, m_Y;
    };

    class WindowCloseEvent : public Event
    {
    public:
        WindowCloseEvent() {}

        EVENT_CLASS_TYPE(WindowClosed)
            EVENT_CLASS_CATEGORY(EventCategory_Application)
    };
}
