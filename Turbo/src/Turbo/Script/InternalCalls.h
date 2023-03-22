#pragma once

namespace Turbo
{
    class InternalCalls
    {
    private:
        static void Init();
        static void RegisterComponents();

        friend class Script;
    };
}
