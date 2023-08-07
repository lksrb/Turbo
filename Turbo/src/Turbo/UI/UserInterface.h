#pragma once

#include "Turbo/Core/Input.h"
#include "Turbo/Event/Event.h"

namespace Turbo 
{
    class UserInterface
    {
    public:
        virtual ~UserInterface() = default;

        static Scope<UserInterface> Create();

        virtual void BeginUI() = 0;
        virtual void EndUI() = 0;

        virtual void OnEvent(Event& e) = 0;

        void SetBlockEvents(bool blockEvents);
    protected:
        bool m_BlockEvents = true;
    };
}
