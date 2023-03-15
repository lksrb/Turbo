#pragma once

#include "MonoForwards.h"

namespace Turbo
{
    class Script
    {
    public:
        static void Init();
        static void Shutdown();
    private:
        static void InitMono();
        static void ShutdownMono();
        static void LoadAssemblies();
        static void ReflectAssembly(MonoAssembly* assembly);

        static MonoAssembly* LoadAssembly(const std::string& path);
    };
}
