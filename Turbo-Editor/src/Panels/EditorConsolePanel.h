#pragma once

#include "Panel.h"

namespace Turbo::Ed
{
    class EditorConsolePanel : public Panel
    {
    public:
        void OnDrawUI() override;
        void OnEvent(Event& e) override;
    private:

    };
}
