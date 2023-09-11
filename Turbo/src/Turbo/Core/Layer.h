#pragma once

#include "Core.h"
#include "Time.h"

namespace Turbo {

    class Event;

    class TBO_NOVTABLE Layer
    {
    public:
        virtual ~Layer() = default;
        virtual void OnAttach() {};
        virtual void OnDetach() {};
        virtual void OnUpdate(Time time) {};
        virtual void OnEvent(Event& event) {};
        virtual void OnDrawUI() {};
    };

}
