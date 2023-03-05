#pragma once

#include <Turbo/Event/Event.h>

namespace Turbo::Ed
{
    class Panel
    {
    public:
        Panel() = default;
        virtual ~Panel() = default;

        virtual void OnDrawUI() = 0;
        virtual void OnEvent(Event& e) = 0;
    private:
    };
}
