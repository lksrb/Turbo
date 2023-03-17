#pragma once

#include "MonoForwards.h"
#include "ScriptClass.h"
#include "ScriptInstance.h"

#include "Turbo/Core/UUID.h"

namespace Turbo
{
    class Scene;
    class Entity;

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

            std::unordered_map<UUID, Ref<ScriptInstance>> ScriptInstances;
            std::unordered_map<std::string, Ref<ScriptClass>> ScriptClasses;

            Scene* SceneContext = nullptr;
        };

        static void Init();
        static void Shutdown();
        static void OnRuntimeStart(Scene* scene);
        static void OnRuntimeStop();
        static void InvokeEntityOnStart(Entity entity);
        static void InvokeEntityOnUpdate(Entity entity, FTime ts);

        static bool ScriptClassExists(const std::string& class_name);

        static Scene* GetCurrentScene();
        static Ref<ScriptInstance> FindEntityInstance(UUID uuid);
    private:
        static void InitMono();
        static void ShutdownMono();
        static void LoadAssemblies();
        static void SetupScriptClasses();
        static void ReflectAssembly(MonoAssembly* assembly);

        static MonoAssembly* LoadAssembly(const std::string& path);
    };
}
