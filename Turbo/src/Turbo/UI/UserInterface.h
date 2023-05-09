#pragma once

#include "Turbo/Core/Input.h"
#include "Turbo/Event/Event.h"

namespace Turbo 
{
    class UserInterface 
    {
    public:
        static Ref<UserInterface> Create();
        virtual ~UserInterface();

        virtual void BeginUI() = 0;
        virtual void EndUI() = 0;

        virtual void OnEvent(Event& e) = 0;

        void SetBlockEvents(bool blockEvents);
    protected:
        UserInterface();

        bool m_BlockEvents = true;
    };
}
