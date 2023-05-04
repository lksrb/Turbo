#include "tbopch.h"
#include "Script.h"

#include "InternalCalls.h"

#include "Turbo/Core/FileSystem.h"
#include "Turbo/Core/Engine.h"

#include "Turbo/Scene/Scene.h"
#include "Turbo/Scene/Entity.h"
#include "Turbo/Scene/Components.h"
#include "Turbo/Solution/Project.h"
#include "Turbo/Physics/Physics2D.h"

#include <fstream>

#include <mono/jit/jit.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/tabledefs.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/mono-gc.h>

namespace Turbo
{
    namespace Utils
    {
        static ScriptFieldType GetFieldType(MonoClassField* field)
        {
            static const std::unordered_map<std::string, ScriptFieldType> s_ScriptFieldTypeMap =
            {
                { "System.Single",  ScriptFieldType::Float },
                { "System.Double",  ScriptFieldType::Double },
                { "System.Boolean", ScriptFieldType::Bool },
                { "System.Char",    ScriptFieldType::Char },
                { "System.Int16",   ScriptFieldType::Short },
                { "System.Int32",   ScriptFieldType::Int },
                { "System.Int64",   ScriptFieldType::Long },
                { "System.SByte",   ScriptFieldType::Byte},
                { "System.Byte",    ScriptFieldType::UByte },
                { "System.UInt16",  ScriptFieldType::UShort },
                { "System.UInt32",  ScriptFieldType::UInt },
                { "System.UInt64",  ScriptFieldType::ULong },
                { "Turbo.Entity",   ScriptFieldType::Entity },
                { "Turbo.Vector2",  ScriptFieldType::Vector2 },
                { "Turbo.Vector3",  ScriptFieldType::Vector3 },
                { "Turbo.Vector4",  ScriptFieldType::Vector4 }
            };

            MonoType* type = mono_field_get_type(field);
            auto& it = s_ScriptFieldTypeMap.find(mono_type_get_name(type));

            if (it != s_ScriptFieldTypeMap.end())
                return it->second;

            return ScriptFieldType::None;
        }

        static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath, bool loadPdb = false)
        {
            ScopedBuffer assemblyData = FileSystem::ReadBinary(assemblyPath);

            MonoImageOpenStatus status;
            MonoImage* image = mono_image_open_from_data_full(assemblyData.As<char>(), static_cast<u32>(assemblyData.Size()), true, &status, false);

            if (status != MONO_IMAGE_OK)
            {
                const char* message = mono_image_strerror(status);
                TBO_ENGINE_ERROR(message);
                return nullptr;
            }

            if (loadPdb)
            {
                std::filesystem::path pdbPath = assemblyPath;
                pdbPath.replace_extension(".pdb");

                if (std::filesystem::exists(pdbPath))
                {
                    ScopedBuffer pdbData = FileSystem::ReadBinary(pdbPath);
                    mono_debug_open_image_from_memory(image, pdbData.As<const mono_byte>(), static_cast<u32>(pdbData.Size()));
                    TBO_ENGINE_INFO("Loaded PDB File! ({})", pdbPath);
                }
            }
            MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.string().c_str(), &status, false);
            mono_image_close(image);

            return assembly;
        }

        static void PrintAssembly(MonoAssembly* assembly)
        {
            MonoImage* image = mono_assembly_get_image(assembly);

            // Classes and namespaces
            {
                const MonoTableInfo* typedefs = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
                i32 num_types = mono_table_info_get_rows(typedefs);

                for (i32 i = 0; i < num_types; i++)
                {
                    u32 cols[MONO_TYPEDEF_SIZE];
                    mono_metadata_decode_row(typedefs, i, cols, MONO_TYPEDEF_SIZE);

                    const char* namespace_name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
                    const char* class_name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

                    const std::string& klass = fmt::format("{0}.{1}", namespace_name, class_name);

                    TBO_ENGINE_TRACE("[Mono-Reflect]: {0}", klass);
                }
            }
        }
    }

    struct Script::Data
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

        std::unordered_map<UUID, ScriptFieldInstanceMap> EntityScriptFieldInstances;
        ScriptClassMap ScriptClasses;

        std::unordered_map<UUID, Ref<ScriptInstance>> ScriptInstances;

        Scene* SceneContext = nullptr;

        Scope<FileWatcher> ProjectPathWatcher;
        bool AssemblyReloadPending = false;
        bool ProjectAssemblyDirty = false;

        bool MonoDebugging = true;
    };

    void Script::Init()
    {
        s_Data = new Script::Data;

        // Initialize mono C# virtual machine
        InitMono();

        // Load script core and project assembly
        LoadCoreAssembly("Resources/Scripts/Turbo-ScriptCore.dll");

        // Register functions
        InternalCalls::Init();

        TBO_ENGINE_INFO("Successfully initialized mono!");
    }

    void Script::OnRuntimeStart(Scene* scene)
    {
        s_Data->SceneContext = scene;

        // Instantiate script instances and sets field instances
        auto& scripts = scene->GetAllEntitiesWith<ScriptComponent, IDComponent>();
        for (auto& e : scripts)
        {
            Entity entity = { e, scene };
            auto& [script, id] = entity.GetComponents<ScriptComponent, IDComponent>();
            UUID uuid = id.ID;

            bool isValidClassName = ScriptClassExists(script.ClassName);

            if (isValidClassName)
            {
                Ref<ScriptClass> scriptClass = s_Data->ScriptClasses.at(script.ClassName);

                Ref<ScriptInstance>& instance = s_Data->ScriptInstances[uuid];
                instance = Ref<ScriptInstance>::Create(scriptClass, uuid);

                // Copy fields
                if (s_Data->EntityScriptFieldInstances.find(uuid) != s_Data->EntityScriptFieldInstances.end())
                {
                    const ScriptFieldInstanceMap& fieldMap = s_Data->EntityScriptFieldInstances.at(uuid);
                    for (const auto& [name, fieldInstance] : fieldMap)
                    {
                        // Because not all script entities exist yet, skip it
                        if (fieldInstance.Field.Type == ScriptFieldType::Entity)
                            continue;

                        instance->SetFieldValueInternal(name, fieldInstance.Buffer);
                    }
                }
            }
        }

        // Loop through all script instances and copy Entity references
        for (auto& [uuid, instance] : s_Data->ScriptInstances)
        {
            const ScriptFieldInstanceMap& fieldMap = s_Data->EntityScriptFieldInstances.at(uuid);

            for (const auto& [name, fieldInstance] : fieldMap)
            {
                if (fieldInstance.Field.Type == ScriptFieldType::Entity)
                {
                    UUID entityRefUUID = fieldInstance.GetValue<UUID>();
                    if (entityRefUUID)
                    {
                        Ref<ScriptInstance> other = FindEntityInstance(entityRefUUID);
                        if (other)
                        {
                            instance->SetFieldValueInternal(name, other->GetMonoInstance());
                        }
                        else
                        {
                            // TODO: What to do if referenced entity does not have a script component
                        }
                    }
                }
            }
        }
    }

    void Script::OnRuntimeStop()
    {
        s_Data->SceneContext = nullptr;
        s_Data->ScriptInstances.clear();

        CollectGarbage();

        if (!Engine::Get().IsClosing() && s_Data->ProjectAssemblyDirty)
        {
            s_Data->ProjectAssemblyDirty = false;
            ReloadAssemblies();
        }
    }

    void Script::CreateScriptInstance(Entity entity)
    {

    }

    void Script::DestroyScriptInstance(Entity entity)
    {
        UUID uuid = entity.GetUUID();

        auto& it = s_Data->ScriptInstances.find(uuid);
        if (it != s_Data->ScriptInstances.end())
        {
            s_Data->ScriptInstances.erase(it);
            return;
        }

        TBO_ENGINE_ERROR("Entity does not have an attached script!");
    }

    void Script::InvokeEntityOnCreate(Entity entity)
    {
        auto& [script, id] = entity.GetComponents<ScriptComponent, IDComponent>();
        UUID uuid = id.ID;

        Ref<ScriptInstance> runtimeInstance = FindEntityInstance(uuid);
        if (!runtimeInstance)
        {
            // Entity instantiated at runtime

            bool isValidClassName = ScriptClassExists(script.ClassName);

            if (isValidClassName)
            {
                Ref<ScriptClass> scriptClass = s_Data->ScriptClasses.at(script.ClassName);

                Ref<ScriptInstance>& instance = s_Data->ScriptInstances[uuid];
                instance = Ref<ScriptInstance>::Create(scriptClass, uuid);

                // Copy fields
                if (s_Data->EntityScriptFieldInstances.find(uuid) != s_Data->EntityScriptFieldInstances.end())
                {
                    const ScriptFieldInstanceMap& fieldMap = s_Data->EntityScriptFieldInstances.at(uuid);
                    for (const auto& [name, fieldInstance] : fieldMap)
                    {
                        if (fieldInstance.Field.Type == ScriptFieldType::Entity)
                        {
                            UUID entityRefUUID = fieldInstance.GetValue<UUID>();
                            Ref<ScriptInstance> other = FindEntityInstance(entityRefUUID);
                            instance->SetFieldValueInternal(name, other->GetMonoInstance());
                            continue;
                        }

                        instance->SetFieldValueInternal(name, fieldInstance.Buffer);
                    }
                }

                runtimeInstance = instance;
            }
        }

        runtimeInstance->InvokeOnCreate();
    }

    void Script::InvokeEntityOnUpdate(Entity entity, FTime ts)
    {
        auto& [script, id] = entity.GetComponents<ScriptComponent, IDComponent>();
        UUID uuid = id.ID;

        Ref<ScriptInstance> instance = FindEntityInstance(uuid);
        if(instance)
            instance->InvokeOnUpdate(ts);
    }

    void Script::InvokeEntityOnBeginCollision2D(Entity entity, Entity other, bool isSensor)
    {
        auto& [script, id] = entity.GetComponents<ScriptComponent, IDComponent>();
        UUID otherUUID = other.GetUUID();
        bool isValidClassName = ScriptClassExists(script.ClassName);

        // Invoke only when C# script class exists
        if (!isValidClassName)
            return;

        if (isSensor)
            s_Data->ScriptInstances.at(id.ID)->InvokeOnTriggerBegin2D(otherUUID);
        else
            s_Data->ScriptInstances.at(id.ID)->InvokeOnCollisionBegin2D(otherUUID);

    }

    void Script::InvokeEntityOnEndCollision2D(Entity entity, Entity other, bool isSensor)
    {
        auto& [script, id] = entity.GetComponents<ScriptComponent, IDComponent>();
        UUID otherUUID = other.GetUUID();
        bool isValidClassName = ScriptClassExists(script.ClassName);

        // Invoke only when C# script class exists
        if (!isValidClassName)
            return;

        if (isSensor)
            s_Data->ScriptInstances.at(id.ID)->InvokeOnTriggerEnd2D(otherUUID);
        else
            s_Data->ScriptInstances.at(id.ID)->InvokeOnCollisionEnd2D(otherUUID);
    }

    Script::ScriptFieldInstanceMap& Script::GetEntityFieldMap(UUID uuid)
    {
        return s_Data->EntityScriptFieldInstances[uuid];
    }

    void Script::LoadProjectAssembly(const std::filesystem::path& path)
    {
        s_Data->ProjectAssemblyPath = path;

        // Clear all registered classes
        s_Data->ScriptClasses.clear();

        s_Data->ProjectAssembly = Utils::LoadMonoAssembly(path, s_Data->MonoDebugging);
        s_Data->ProjectAssemblyImage = mono_assembly_get_image(s_Data->ProjectAssembly);

        ReflectProjectAssembly();

        // Start watching the path
        s_Data->ProjectPathWatcher = CreateScope<FileWatcher>(FileWatcher::NotifyEvent_All, false, Script::OnProjectDirectoryChange);
        s_Data->ProjectPathWatcher->Watch(s_Data->ProjectAssemblyPath.parent_path());
        s_Data->AssemblyReloadPending = false;

        CollectGarbage();
    }

    void Script::ReflectProjectAssembly()
    {
        Utils::PrintAssembly(s_Data->ProjectAssembly);

        // Reflection
        {
            const MonoTableInfo* typedefTable = mono_image_get_table_info(s_Data->ProjectAssemblyImage, MONO_TABLE_TYPEDEF);
            i32 numTypes = mono_table_info_get_rows(typedefTable);

            // Iterate through all classes
            for (i32 i = 0; i < numTypes; ++i)
            {
                u32 cols[MONO_TYPEDEF_SIZE];
                mono_metadata_decode_row(typedefTable, i, cols, MONO_TYPEDEF_SIZE);

                const char* className = mono_metadata_string_heap(s_Data->ProjectAssemblyImage, cols[MONO_TYPEDEF_NAME]);
                const char* nameSpace = mono_metadata_string_heap(s_Data->ProjectAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
                std::string fullName;

                // Use different formatting when dealing with namespace-less assemblies
                if (strlen(nameSpace) != 0)
                    fullName = fmt::format("{}.{}", nameSpace, className);
                else
                    fullName = className;

                // Is null when not a class type
                MonoClass* klass = mono_class_from_name(s_Data->ProjectAssemblyImage, nameSpace, className);

                // If is not a class, skip
                if (klass == nullptr)
                    continue;

                // If class is entity class, skip
                if (s_Data->EntityBaseClass->GetMonoClass() == klass)
                    continue;

                // If entity class is not the base of the class, skip
                bool hasEntityBase = mono_class_is_subclass_of(klass, s_Data->EntityBaseClass->GetMonoClass(), false);
                if (hasEntityBase == false)
                    continue;

                Ref<ScriptClass>& scriptClass = s_Data->ScriptClasses[fullName];
                scriptClass = Ref<ScriptClass>::Create(klass, s_Data->EntityBaseClass);

                // Iterate through all fields in a class

                void* iter = nullptr;
                while (MonoClassField* monoField = mono_class_get_fields(klass, &iter))
                {
                    const char* fieldName = mono_field_get_name(monoField);
                    u32 accessFlags = mono_field_get_flags(monoField);

                    if (accessFlags & MONO_FIELD_ATTR_PUBLIC)
                    {
                        ScriptField& field = scriptClass->m_ScriptFields[fieldName];
                        field.MonoField = monoField;
                        field.Type = Utils::GetFieldType(monoField);

                        TBO_ENGINE_TRACE("[Mono-Reflect-ClassFields]: public [FieldType] {0}", fieldName);
                    }
                }
            }
        }
    }

    // Callback from FileWatcher
    void Script::OnProjectDirectoryChange(std::filesystem::path path, FileWatcher::FileEvent event)
    {
        // NOTE: Without 'AssemblyReloadPending' this method is called twice when the project is built
        if (!Script::s_Data->AssemblyReloadPending && event == FileWatcher::FileEvent_Modified && path.stem() == Project::GetProjectName() && path.extension() == ".dll")
        {
            // After scene runtime stops, reload assemblies automatically if dirty
            if (s_Data->SceneContext)
            {
                s_Data->ProjectAssemblyDirty = true;
                return;
            }

            // Project assembly reloaded
            s_Data->AssemblyReloadPending = true;

            Engine::Get().SubmitToMainThread([event, &path]()
            {
                s_Data->ProjectPathWatcher.reset();

                Script::ReloadAssemblies();
            });
        }
    }

    bool Script::ScriptClassExists(const std::string& class_name)
    {
        auto& it = s_Data->ScriptClasses.find(class_name);

        if (it == s_Data->ScriptClasses.end())
            return false;

        return true;
    }

    void Script::InitMono()
    {
        // Set path for important C# assemblies
        mono_set_assemblies_path("Mono/lib");

        // Debugging
        if (s_Data->MonoDebugging)
        {
            const char* argv[2] = {
                "--debugger-agent=transport=dt_socket,address=127.0.0.1:2550,server=y,suspend=n,loglevel=3,logfile=MonoDebugger.log",
                "--soft-breakpoints"
            };

            mono_jit_parse_options(2, (char**)argv);
            mono_debug_init(MONO_DEBUG_FORMAT_MONO);
        }

        // Create root domain
        s_Data->RootDomain = mono_jit_init("TBOJITRuntime");
        TBO_ENGINE_ASSERT(s_Data->RootDomain, "Could not initialize mono runtime!");

        if (s_Data->MonoDebugging)
            mono_debug_domain_create(s_Data->RootDomain);

        mono_thread_set_main(mono_thread_current());
    }

    void Script::LoadCoreAssembly(const std::filesystem::path& path)
    {
        // Create App Domain
        s_Data->AppDomain = mono_domain_create_appdomain("TBORuntime", nullptr);
        mono_domain_set(s_Data->AppDomain, true);

        s_Data->CoreAssemblyPath = path;
        s_Data->ScriptCoreAssembly = Utils::LoadMonoAssembly(path, s_Data->MonoDebugging);
        s_Data->ScriptCoreAssemblyImage = mono_assembly_get_image(s_Data->ScriptCoreAssembly);

        s_Data->EntityBaseClass = Ref<ScriptClass>::Create("Turbo", "Entity");

        Utils::PrintAssembly(s_Data->ScriptCoreAssembly);
    }

    void Script::CollectGarbage()
    {
        TBO_ENGINE_WARN("Collecting garbage...");

        // Collect garbage
        mono_gc_collect(mono_gc_max_generation());

        // Block until finalized
        while (mono_gc_pending_finalizers());

        TBO_ENGINE_WARN("GC Finished...");
    }

    void Script::ReloadAssemblies()
    {
        TBO_ENGINE_ASSERT(!s_Data->CoreAssemblyPath.empty());
        TBO_ENGINE_ASSERT(!s_Data->ProjectAssemblyPath.empty());

        CollectGarbage();

        // Unloading
        {
            // Switch away from the reloaded app domain
            mono_domain_set(s_Data->RootDomain, false);
            mono_domain_unload(s_Data->AppDomain);
        }

        LoadCoreAssembly(s_Data->CoreAssemblyPath);
        LoadProjectAssembly(s_Data->ProjectAssemblyPath);

        InternalCalls::RegisterComponents();

        TBO_ENGINE_INFO("Assembly successfully reloaded!");
    }

    void Script::Shutdown()
    {
        CollectGarbage();

        ShutdownMono();

        delete s_Data;
        s_Data = nullptr;
    }

    void Script::ShutdownMono()
    {
        mono_domain_set(s_Data->RootDomain, false);

        mono_domain_unload(s_Data->AppDomain);
        s_Data->AppDomain = nullptr;

        mono_jit_cleanup(s_Data->RootDomain);
        s_Data->RootDomain = nullptr;
    }

    Scene* Script::GetCurrentScene()
    {
        return s_Data->SceneContext;
    }

    MonoImage* Script::GetCoreAssemblyImage()
    {
        return s_Data->ScriptCoreAssemblyImage;
    }

    MonoDomain* Script::GetAppDomain()
    {
        return s_Data->AppDomain;
    }

    const Script::ScriptClassMap& Script::GetScriptClassMap()
    {
        return s_Data->ScriptClasses;
    }

    UUID Script::GetUUIDFromMonoObject(MonoObject* instance)
	{
        if (instance)
        {
            for (auto& [ID, script] : s_Data->ScriptInstances)
            {
                if (script->GetMonoInstance() == instance)
                    return ID;
            }
        }

        return 0;
	}

	Ref<ScriptInstance> Script::FindEntityInstance(UUID uuid)
    {
        auto& it = s_Data->ScriptInstances.find(uuid);

        if (it == s_Data->ScriptInstances.end())
            return nullptr;

        return it->second;
    }

    Ref<ScriptClass> Script::FindEntityClass(const std::string& name)
    {
        auto& it = s_Data->ScriptClasses.find(name);

        if (it == s_Data->ScriptClasses.end())
            return nullptr;

        return it->second;
    }

	Ref<ScriptClass> Script::GetEntityBaseClass()
	{
        return s_Data->EntityBaseClass;
	}

}
