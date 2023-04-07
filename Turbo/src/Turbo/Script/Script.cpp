#include "tbopch.h"
#include "Script.h"

#include "InternalCalls.h"
#include "ScriptInstance.h"
#include "ScriptField.h"

#include "Turbo/Core/FileSystem.h"
#include "Turbo/Core/Engine.h"
#include "Turbo/Scene/Scene.h"
#include "Turbo/Scene/Entity.h"
#include "Turbo/Scene/Components.h"
#include "Turbo/Solution/Project.h"

#include <fstream>

#include <mono/jit/jit.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/tabledefs.h>
#include <mono/metadata/attrdefs.h>

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

    extern Script::Data* g_Data = nullptr;

    void Script::Init()
    {
        g_Data = new Script::Data;

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
        g_Data->SceneContext = scene;
    }

    void Script::OnRuntimeStop()
    {
        g_Data->SceneContext = nullptr;
        g_Data->ScriptInstances.clear();

        if (g_Data->ProjectAssemblyDirty)
        {
            g_Data->ProjectAssemblyDirty = false;
            ReloadAssemblies();
        }
    }

    void Script::InvokeEntityOnStart(Entity entity)
    {
        auto& [script, id] = entity.GetComponents<ScriptComponent, IDComponent>();
        UUID uuid = id.ID;

        bool isValidClassName = ScriptClassExists(script.ClassName);

        if (isValidClassName)
        {
            Ref<ScriptClass> script_class = g_Data->ScriptClasses.at(script.ClassName);

            Ref<ScriptInstance>& instance = g_Data->ScriptInstances[uuid];
            instance = Ref<ScriptInstance>::Create(script_class, uuid);

            // Copy fields
            if (g_Data->EntityScriptFieldInstances.find(uuid) != g_Data->EntityScriptFieldInstances.end())
            {
                const ScriptFieldInstanceMap& fieldMap = g_Data->EntityScriptFieldInstances.at(uuid);
                for (const auto& [name, fieldInstance] : fieldMap)
                {
                    instance->SetFieldValueInternal(name, fieldInstance.Buffer);
                }
            }

            instance->InvokeOnStart();
        }
    }

    void Script::InvokeEntityOnUpdate(Entity entity, FTime ts)
    {
        auto& [script, id] = entity.GetComponents<ScriptComponent, IDComponent>();
        bool isValidClassName = ScriptClassExists(script.ClassName);

        if (isValidClassName)
        {
            g_Data->ScriptInstances.at(id.ID)->InvokeOnUpdate(ts);
        }
    }

    Script::ScriptFieldInstanceMap& Script::GetEntityFieldMap(UUID uuid)
    {
        return g_Data->EntityScriptFieldInstances[uuid];
    }

    void Script::LoadProjectAssembly(const std::filesystem::path& path)
    {
        g_Data->ProjectAssemblyPath = path;

        // Clear all registered classes
        g_Data->ScriptClasses.clear();

        g_Data->ProjectAssembly = Utils::LoadMonoAssembly(path, g_Data->MonoDebugging);
        g_Data->ProjectAssemblyImage = mono_assembly_get_image(g_Data->ProjectAssembly);

        ReflectProjectAssembly();

        // Start watching the path
        g_Data->ProjectPathWatcher = CreateScope<FileWatcher>(FileWatcher::NotifyEvent_All, false, Script::OnProjectDirectoryChange);
        g_Data->ProjectPathWatcher->Watch(g_Data->ProjectAssemblyPath.parent_path());
        g_Data->AssemblyReloadPending = false;
    }

    void Script::ReflectProjectAssembly()
    {
        Utils::PrintAssembly(g_Data->ProjectAssembly);

        // Reflection
        {
            const MonoTableInfo* typedef_table = mono_image_get_table_info(g_Data->ProjectAssemblyImage, MONO_TABLE_TYPEDEF);
            i32 num_types = mono_table_info_get_rows(typedef_table);

            // Iterate through all classes
            for (i32 i = 0; i < num_types; ++i)
            {
                u32 cols[MONO_TYPEDEF_SIZE];
                mono_metadata_decode_row(typedef_table, i, cols, MONO_TYPEDEF_SIZE);

                const char* class_name = mono_metadata_string_heap(g_Data->ProjectAssemblyImage, cols[MONO_TYPEDEF_NAME]);
                const char* namespace_name = mono_metadata_string_heap(g_Data->ProjectAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
                std::string full_name;

                // Use different formatting when dealing with namespace-less assemblies
                if (strlen(namespace_name) != 0)
                    full_name = fmt::format("{}.{}", namespace_name, class_name);
                else
                    full_name = class_name;

                // Is null when not a class type
                MonoClass* klass = mono_class_from_name(g_Data->ProjectAssemblyImage, namespace_name, class_name);

                // If is not a class, skip
                if (klass == nullptr)
                    continue;

                // If class is entity class, skip
                if (g_Data->EntityBaseClass->GetMonoClass() == klass)
                    continue;

                // If entity class is not the base of the class, skip
                bool has_entity_base = mono_class_is_subclass_of(klass, g_Data->EntityBaseClass->GetMonoClass(), false);
                if (has_entity_base == false)
                    continue;

                Ref<ScriptClass>& script_class = g_Data->ScriptClasses[full_name];
                script_class = Ref<ScriptClass>::Create(klass, g_Data->EntityBaseClass);

                // Iterate through all fields in a class

                void* iter = nullptr;
                while (MonoClassField* mono_field = mono_class_get_fields(klass, &iter))
                {
                    const char* field_name = mono_field_get_name(mono_field);
                    u32 access_flags = mono_field_get_flags(mono_field);

                    if (access_flags & MONO_FIELD_ATTR_PUBLIC)
                    {
                        ScriptField& field = script_class->m_ScriptFields[field_name];
                        field.MonoField = mono_field;
                        field.Type = Utils::GetFieldType(mono_field);

                        TBO_ENGINE_TRACE("[Mono-Reflect-ClassFields]: public [FieldType] {0}", field_name);
                    }
                }
            }
        }
    }

    bool Script::ScriptClassExists(const std::string& class_name)
    {
        auto& it = g_Data->ScriptClasses.find(class_name);

        if (it == g_Data->ScriptClasses.end())
            return false;

        return true;
    }

    void Script::InitMono()
    {
        // Set path for important C# assemblies
        mono_set_assemblies_path("Mono/lib");

        // Debugging
        if (g_Data->MonoDebugging)
        {
            const char* argv[2] = {
                "--debugger-agent=transport=dt_socket,address=127.0.0.1:2550,server=y,suspend=n,loglevel=3,logfile=MonoDebugger.log",
                "--soft-breakpoints"
            };

            mono_jit_parse_options(2, (char**)argv);
            mono_debug_init(MONO_DEBUG_FORMAT_MONO);
        }

        // Create root domain
        g_Data->RootDomain = mono_jit_init("TBOJITRuntime");
        TBO_ENGINE_ASSERT(g_Data->RootDomain, "Could not initialize mono runtime!");

        if (g_Data->MonoDebugging)
            mono_debug_domain_create(g_Data->RootDomain);

        mono_thread_set_main(mono_thread_current());
    }

    void Script::LoadCoreAssembly(const std::filesystem::path& path)
    {
        // Create App Domain
        g_Data->AppDomain = mono_domain_create_appdomain("TBORuntime", nullptr);
        mono_domain_set(g_Data->AppDomain, true);

        g_Data->CoreAssemblyPath = path;
        g_Data->ScriptCoreAssembly = Utils::LoadMonoAssembly(path, g_Data->MonoDebugging);
        g_Data->ScriptCoreAssemblyImage = mono_assembly_get_image(g_Data->ScriptCoreAssembly);

        g_Data->EntityBaseClass = Ref<ScriptClass>::Create("Turbo", "Entity");

        Utils::PrintAssembly(g_Data->ScriptCoreAssembly);
    }

    // Callback from filewatcher
    void Script::OnProjectDirectoryChange(std::filesystem::path path, FileWatcher::FileEvent e)
    {
        // NOTE: Without 'AssemblyReloadPending' this method is called twice when the project is built
        if (!g_Data->AssemblyReloadPending && e == FileWatcher::FileEvent_Modified && path.stem() == Project::GetProjectName() && path.extension() == ".dll")
        {
            // After scene runtime stops, reload assemblies automatically if dirty
            if (g_Data->SceneContext)
            {
                g_Data->ProjectAssemblyDirty = true;
                return;
            }

            // Project assembly reloaded
            g_Data->AssemblyReloadPending = true;

            Engine::Get().SubmitToMainThread([e, &path]() 
            {
                g_Data->ProjectPathWatcher.reset();

                Script::ReloadAssemblies();
            });
        }
    }

    void Script::ReloadAssemblies()
    {
        TBO_ENGINE_ASSERT(!g_Data->CoreAssemblyPath.empty());
        TBO_ENGINE_ASSERT(!g_Data->ProjectAssemblyPath.empty());

        // Unloading
        {
            // Switch away from the reloaded app domain
            mono_domain_set(g_Data->RootDomain, false);
            mono_domain_unload(g_Data->AppDomain);
        }

        LoadCoreAssembly(g_Data->CoreAssemblyPath);
        LoadProjectAssembly(g_Data->ProjectAssemblyPath);

        InternalCalls::RegisterComponents();

        TBO_ENGINE_INFO("Assembly successfully reloaded!");
    }

    void Script::Shutdown()
    {
        ShutdownMono();

        delete g_Data;
        g_Data = nullptr;
    }

    void Script::ShutdownMono()
    {
        mono_domain_set(g_Data->RootDomain, false);

        mono_domain_unload(g_Data->AppDomain);
        g_Data->AppDomain = nullptr;

        mono_jit_cleanup(g_Data->RootDomain);
        g_Data->RootDomain = nullptr;
    }

    Scene* Script::GetCurrentScene()
    {
        return g_Data->SceneContext;
    }

    Ref<ScriptInstance> Script::FindEntityInstance(UUID uuid)
    {
        auto& it = g_Data->ScriptInstances.find(uuid);

        if (it == g_Data->ScriptInstances.end())
            return nullptr;

        return it->second;
    }

    Ref<ScriptClass> Script::FindEntityClass(const std::string& name)
    {
        auto& it = g_Data->ScriptClasses.find(name);

        if (it == g_Data->ScriptClasses.end())
            return nullptr;

        return it->second;
    }

}
