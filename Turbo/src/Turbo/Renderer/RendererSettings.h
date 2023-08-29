#pragma once

#include "Turbo/Core/PrimitiveTypes.h"

namespace Turbo {

    struct RendererCapabilities
    {
        // TODO
    };

    namespace RendererSettings
    {
        inline constexpr u32 FramesInFlight = 2;
        inline constexpr bool EnableValidationLayers = true;
    }

}
