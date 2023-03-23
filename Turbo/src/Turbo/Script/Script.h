#pragma once

#include "ScriptClass.h"
#include "ScriptField.h"
#include "ScriptInstance.h"

#include "Turbo/Core/UUID.h"
#include "Turbo/Core/FileWatcher.h"

namespace Turbo
{
    class Scene;
    class Entity;

    class Script
    {
    public:
        using ScriptFieldInstanceMap = std::unordered_map<std::string, ScriptFieldInstance>;

        struct Data
        {
            MonoDomain* RootDomain = nullptr;
            MonoDomain* AppDomain = nullptr;
            MonoAssembly* ScriptCoreAssembly = nullptr;
            MonoImage* ScriptCoreAssemblyImage = nullptr;

            MonoAssembly* ProjectAssembly = nullptr;
            MonoImage* ProjectAssemblyImage = nullptr;

            Ref<ScriptClass> EntityBaseClass;

            std::filesystem::path CoreAssemblyPath;
            std::filesystem::path ProjectAssemblyPath;

            std::unordered_map<UUID, Ref<ScriptInstance>> ScriptInstances;
            std::unordered_map<std::string, Ref<ScriptClass>> ScriptClasses;
            std::unordered_map<UUID, ScriptFieldInstanceMap> EntityScriptFieldInstances;

            Scene* SceneContext = nullptr;

            Scope<FileWatcher> ProjectPathWatcher;
            bool AssemblyReloadPending = false;
            bool ProjectAssemblyDirty = false;
        };

        static void Init();
        static void Shutdown();
        static void OnRuntimeStart(Scene* scene);
        static void OnRuntimeStop();
        static void InvokeEntityOnStart(Entity entity);
        static void InvokeEntityOnUpdate(Entity entity, FTime ts);

        static ScriptFieldInstanceMap& GetEntityFieldMap(UUID uuid);

        static void ReloadAssemblies();
        static void LoadProjectAssembly(const std::filesystem::path& path);

        static bool ScriptClassExists(const std::string& class_name);

        static Scene* GetCurrentScene();
        static Ref<ScriptInstance> FindEntityInstance(UUID uuid);
        static Ref<ScriptClass> FindEntityClass(const std::string& name);
    private:
        static void OnProjectDirectoryChange(std::filesystem::path path, FileWatcher::FileEvent e);
        static void InitMono();
        static void ShutdownMono();
        static void LoadCoreAssembly(const std::filesystem::path& path);
        static void ReflectProjectAssembly();
    };
}
