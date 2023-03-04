#pragma once

#include "Turbo/Event/Event.h"

namespace Turbo 
{
    class UserInterface 
    {
    public:
        static Ref<UserInterface>Create();
        virtual ~UserInterface();

        virtual void BeginUI() = 0;
        virtual void EndUI() = 0;

        virtual void OnEvent(Event& e) = 0;

        void SetBlockEvents(bool block_events);
    protected:
        UserInterface();

        bool m_BlockEvents;
    };
}
