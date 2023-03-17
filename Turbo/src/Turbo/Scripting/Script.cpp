#include "tbopch.h"
#include "Script.h"

#include "InternalCalls.h"
#include "ScriptInstance.h"
#include "Turbo/Scene/Scene.h"
#include "Turbo/Scene/Entity.h"
#include "Turbo/Scene/Components.h"

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
        LoadAssemblies();

        // Register functions
        InternalCalls::Init();

        // Setup script classes
        SetupScriptClasses();

/*
        // Test class method invocation
        {
            MonoImage* image = mono_assembly_get_image(g_Data->ScriptCoreAssembly);
            MonoClass* klass = mono_class_from_name(image, "Turbo", "ScriptCoreTest");

            // Allocate an instance of a class
            MonoObject* class_instance = mono_object_new(g_Data->AppDomain, klass);
            TBO_ENGINE_ASSERT(class_instance, "Could not allocate memory for a class instance!");

            // Call the parameterless constructor
            mono_runtime_object_init(class_instance);

            // Get a reference to the method in the class
            MonoMethod* method = mono_class_get_method_from_name(klass, "PrintHello", 0);
            TBO_ENGINE_ASSERT(method, "Could not find the method!");

            // Call C# method on the object instance
            MonoObject* exception = nullptr;
            mono_runtime_invoke(method, class_instance, nullptr, &exception);
        }*/

        TBO_ENGINE_INFO("Successfully initialized mono!");
    }

    MonoAssembly* Script::LoadAssembly(const std::string& assembly_path)
    {
        u32 bytes_size;
        char* bytes = Utils::ReadBytes(assembly_path, &bytes_size);

        MonoImageOpenStatus status;
        MonoImage* image = mono_image_open_from_data_full(bytes, bytes_size, true, &status, false);

        TBO_ENGINE_ASSERT(status == MONO_IMAGE_OK, "Could not load the assembly image!");

        MonoAssembly* assembly = mono_assembly_load_from_full(image, assembly_path.c_str(), &status, false);
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

    void Script::LoadAssemblies()
    {
        g_Data->ScriptCoreAssembly = LoadAssembly("Resources/Scripts/Turbo-ScriptCore.dll");
        g_Data->ScriptCoreAssemblyImage = mono_assembly_get_image(g_Data->ScriptCoreAssembly);

        ReflectAssembly(g_Data->ScriptCoreAssembly);
        // TODO: Project assembly
    }

    void Script::SetupScriptClasses()
    {
        g_Data->EntityBaseClass = Ref<ScriptClass>::Create("Turbo", "Entity");

        g_Data->ScriptClasses["Turbo.ScriptCoreTest"] = Ref<ScriptClass>::Create("Turbo", "ScriptCoreTest", g_Data->EntityBaseClass);

        //ScriptInstance instance(klass, 456);
        //
        //instance.InvokeOnStart();
        //instance.InvokeOnUpdate(4);
    }

    void Script::ReflectAssembly(MonoAssembly* assembly)
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

                TBO_ENGINE_INFO(klass);
            }
        }

        // Classes and methods
        {
            // TODO:
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
