#pragma once

#include "Turbo/Core/common.h"

namespace Turbo {

    enum class EventType 
    {
        none = 0,
        WindowClosed, WindowResize, WindowFocus, WindowKillFocus, WindowMoved,
        KeyPressed, KeyReleased, KeyTyped,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
    };

    enum EventCategory_ : u32 
    {
        EventCategory_None          = 0,
        EventCategory_Application   = TBO_BIT(0),
        EventCategory_Input         = TBO_BIT(1),
        EventCategory_Keyboard      = TBO_BIT(2),
        EventCategory_Mouse         = TBO_BIT(3),
        EventCategory_MouseButton   = TBO_BIT(4)
    };

    //using EventCategory = uint32; // => EventCategory_

    typedef u32 EventCategory;

#define EVENT_CLASS_TYPE(type)  static EventType GetStaticType() { return EventType::type; }\
								virtual EventType GetEventType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

    class Event 
    {
    public:
        virtual ~Event() = default;

        virtual EventType GetEventType() const = 0;
        virtual const char* GetName() const = 0;
        virtual int GetCategoryFlags() const = 0;
        virtual std::string ToString() const { return GetName(); }

        bool IsInCategory(EventCategory category) const
        {
            return GetCategoryFlags() & category;
        }
    public:
        bool Handled = false;
    };

    class EventDispatcher 
    {
    public:
        EventDispatcher(Event& event)
            : m_Event(event)
        {
        }

        template<typename T, typename F>
        bool Dispatch(const F& func)
        {
            if (m_Event.GetEventType() == T::GetStaticType())
            {
                m_Event.Handled |= func(static_cast<T&>(m_Event));
                return true;
            }
            return false;
        }
    private:
        Event& m_Event;
    };

    inline std::ostream& operator<<(std::ostream& os, const Event& e)
    {
        return os << e.ToString();
    }
}
