#pragma once

#include "ScriptClass.h"
#include "ScriptInstance.h"
#include "ScriptField.h"

#include "Turbo/Core/UUID.h"
#include "Turbo/Core/FileWatcher.h"

namespace Turbo
{
    class Scene;
    class Entity;

    class Script
    {
    public:
        struct Data;

        using ScriptFieldInstanceMap = std::unordered_map<std::string, ScriptFieldInstance>;
        using ScriptClassMap = std::unordered_map<std::string, Ref<ScriptClass>>;

        static void Init();
        static void Shutdown();
        static void OnRuntimeStart(const Ref<Scene>& context);
        static void OnRuntimeStop();

        static void CreateScriptInstance(Entity entity);
        static void DestroyScriptInstance(Entity entity);

        static void InvokeEntityOnCreate(Entity entity);
        static void InvokeEntityOnUpdate(Entity entity);

        static void InvokeEntityOnBeginCollision2D(Entity entity, Entity other, bool isTrigger);
        static void InvokeEntityOnEndCollision2D(Entity entity, Entity other, bool isTrigger);
        static ScriptFieldInstanceMap& GetEntityFieldMap(UUID uuid);

        static void ReloadAssemblies();
        static void LoadProjectAssembly(const std::filesystem::path& path);

        static bool ScriptClassExists(const std::string& className);

        static Ref<Scene> GetCurrentScene();

        static MonoImage* GetCoreAssemblyImage();
        static MonoDomain* GetAppDomain();

        static void OnNewFrame(FTime ts);

        static void CopyScriptClassFields(Entity source, Entity destination);
        static const ScriptClassMap& GetScriptClassMap();
        static UUID GetUUIDFromMonoObject(MonoObject* instance);
        static Ref<ScriptInstance> FindEntityInstance(UUID uuid);
        static Ref<ScriptClass> FindEntityClass(const std::string& name);
        static Ref<ScriptClass> GetEntityBaseClass();
    private:
        static void CollectGarbage();
        static void InitMono();
        static void ShutdownMono();
        static void LoadCoreAssembly(const std::filesystem::path& path);
        static void ReflectProjectAssembly();
        static void OnProjectDirectoryChange(std::filesystem::path path, FileWatcher::FileEvent event);
    private:
        static inline Script::Data* s_Data = nullptr;
    };
}
