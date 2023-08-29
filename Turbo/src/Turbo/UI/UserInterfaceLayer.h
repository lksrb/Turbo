#pragma once

#include "Turbo/Core/Layer.h"

namespace Turbo {

    class UserInterfaceLayer : public Layer
    {
    public:
        static UserInterfaceLayer* Create();

        virtual void Begin() = 0;
        virtual void End() = 0;

        void SetBlockEvents(bool blockEvents);
    protected:
        bool m_BlockEvents = true;
    };
}
