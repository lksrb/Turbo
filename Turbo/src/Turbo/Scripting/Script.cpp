#include "tbopch.h"
#include "Script.h"

#include "InternalCalls.h"
#include "ScriptInstance.h"

#include "Turbo/Scene/Scene.h"
#include "Turbo/Scene/Entity.h"
#include "Turbo/Scene/Components.h"

#include "Turbo/Solution/Project.h"

#include <fstream>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

#include <mono/metadata/appdomain.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/tabledefs.h>
#include <mono/metadata/attrdefs.h>

namespace Turbo
{
    namespace Utils
    {
        static char* ReadBytes(const std::string& path, u32* out_size)
        {
            // std::ios::ate - seeks end of file
            std::ifstream stream(path, std::ios::binary | std::ios::ate);

            if (!stream)
                return nullptr;

            std::streampos end = stream.tellg();
            stream.seekg(0, std::ios::beg);
            size_t size = end - stream.tellg();
            if (size == 0)
                return nullptr;

            char* bytes = new char[size];
            stream.read(bytes, size);
            stream.close();

            *out_size = static_cast<u32>(size);

            return bytes;
        }
    }

    extern Script::Data* g_Data = nullptr;

    void Script::Init()
    {
        g_Data = new Script::Data;

        // Initialize mono C# virtual machine
        InitMono();

        // Load script core and project assembly
        LoadCoreAssembly();

        // Register functions
        InternalCalls::Init();

        TBO_ENGINE_INFO("Successfully initialized mono!");
    }

    MonoAssembly* Script::LoadAssembly(const std::filesystem::path& assembly_path)
    {
        u32 bytes_size;
        char* bytes = Utils::ReadBytes(assembly_path.string(), &bytes_size);

        MonoImageOpenStatus status;
        MonoImage* image = mono_image_open_from_data_full(bytes, bytes_size, true, &status, false);

        TBO_ENGINE_ASSERT(status == MONO_IMAGE_OK, "Could not load the assembly image!");

        MonoAssembly* assembly = mono_assembly_load_from_full(image, assembly_path.string().c_str(), &status, false);
        mono_image_close(image);

        delete[] bytes;

        return assembly;
    }

    void Script::OnRuntimeStart(Scene* scene)
    {
        g_Data->SceneContext = scene;
    }

    void Script::OnRuntimeStop()
    {
        g_Data->SceneContext = nullptr;

        g_Data->ScriptInstances.clear();
    }

    void Script::InvokeEntityOnStart(Entity entity)
    {
        auto& [script, id] = entity.GetComponents<ScriptComponent, IDComponent>();

        Ref<ScriptClass> script_class = g_Data->ScriptClasses.at(script.ClassName);

        Ref<ScriptInstance>& instance = g_Data->ScriptInstances[id.ID];
        instance = Ref<ScriptInstance>::Create(script_class, id.ID);
        instance->InvokeOnStart();
    }

    void Script::InvokeEntityOnUpdate(Entity entity, FTime ts)
    {
        auto& id = entity.GetComponents<IDComponent>();
        g_Data->ScriptInstances.at(id.ID)->InvokeOnUpdate(ts);
    }

    void Script::LoadProjectAssembly(const std::filesystem::path& path)
    {
        if (g_Data->ProjectAssembly)
        {
            // TODO: Unload

            // Clear all registered classes
            g_Data->ScriptClasses.clear();
        }

        g_Data->ProjectAssembly = LoadAssembly(path);
        g_Data->ProjectAssemblyImage = mono_assembly_get_image(g_Data->ProjectAssembly);

        ReflectProjectAssembly();
    }

    void Script::ReflectProjectAssembly()
    {
        PrintAssembly(g_Data->ProjectAssembly);

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

                g_Data->ScriptClasses[full_name] = Ref<ScriptClass>::Create(klass, g_Data->EntityBaseClass);
                /*
                // Iterate through all fields in a class

                void* iter = nullptr;
                while (MonoClassField* monoField = mono_class_get_fields(klass, &iter))
                {
                    const char* fieldName = mono_field_get_name(monoField);
                    ClassFieldType fieldType = UTILS::GetFieldValueType(monoField);
                    ClassFieldAccessFlags accessFlag = UTILS::GetFieldAccessMask(monoField);

                    if (accessFlag & ClassFieldAccessFlags_Public)
                    {
                        auto& field = scriptClass->m_ClassFieldMap[fieldName];
                        field.MonoField = monoField;
                        field.Type = fieldType;

                        PG_CORE_TRACE("[ScriptEngine][Mono] {0}", fieldName);
                    }
                }*/
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

        // Create root domain
        g_Data->RootDomain = mono_jit_init("TBOJITRuntime");
        TBO_ENGINE_ASSERT(g_Data->RootDomain, "Could not initialize mono runtime!");

        // Set main thread as current thread
        mono_thread_set_main(mono_thread_current());

        // Create App Domain
        g_Data->AppDomain = mono_domain_create_appdomain("TBOAppDomain", nullptr);
        TBO_ENGINE_ASSERT(g_Data->AppDomain, "Could not create mono app domain!");

        // Set it as current app domain
        mono_domain_set(g_Data->AppDomain, true);
    }

    void Script::LoadCoreAssembly()
    {
        g_Data->ScriptCoreAssembly = LoadAssembly("Resources/Scripts/Turbo-ScriptCore.dll");
        g_Data->ScriptCoreAssemblyImage = mono_assembly_get_image(g_Data->ScriptCoreAssembly);

        g_Data->EntityBaseClass = Ref<ScriptClass>::Create("Turbo", "Entity");

        PrintAssembly(g_Data->ScriptCoreAssembly);
    }

    void Script::PrintAssembly(MonoAssembly* assembly)
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

                TBO_ENGINE_INFO("[Mono-Reflect]: {0}", klass);
            }
        }
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

        mono_jit_cleanup(g_Data->RootDomain);
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

}
