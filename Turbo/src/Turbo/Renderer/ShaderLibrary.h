#pragma once

#include "Shader.h"

#include <string_view>

namespace Turbo
{
    class ShaderLibrary
    {
    public:
        static void Init();
        static void Shutdown();

        static Ref<Shader> Get(std::string_view shaderName);
    private:
    };
}
