#pragma once

namespace Turbo {

    class RendererContext
    {
    public:
        virtual ~RendererContext() = default;
        virtual void Initialize() = 0;
        virtual void Shutdown() = 0;

        static RendererContext* Create();
    };
}
