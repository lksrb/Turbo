#pragma once

#include "MonoForwards.h"
#include "ScriptClass.h"

namespace Turbo
{
    class Scene;

    class Script
    {
    public:
        struct Data
        {
            MonoDomain* RootDomain = nullptr;
            MonoDomain* AppDomain = nullptr;
            MonoAssembly* ScriptCoreAssembly = nullptr;
            MonoImage* ScriptCoreAssemblyImage = nullptr;

            Ref<ScriptClass> EntityBaseClass;

            Scene* SceneContext = nullptr;
        };

        static void Init();
        static void Shutdown();
    private:
        static Scene* GetCurrentScene();
        static void InitMono();
        static void ShutdownMono();
        static void LoadAssemblies();
        static void SetupScriptClasses();
        static void ReflectAssembly(MonoAssembly* assembly);

        static MonoAssembly* LoadAssembly(const std::string& path);
    };
}
