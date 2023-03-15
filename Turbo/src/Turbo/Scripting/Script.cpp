#include "tbopch.h"
#include "Script.h"

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

    struct ScriptEngineData
    {
        MonoDomain* RootDomain = nullptr;
        MonoDomain* AppDomain = nullptr;
        MonoAssembly* ScriptCoreAssembly = nullptr;
        MonoImage* ScriptCoreAssemblyImage = nullptr;
    };

    extern ScriptEngineData* s_Data = nullptr;

    void Script::Init()
    {
        s_Data = new ScriptEngineData;

        // Initialize mono C# virtual machine
        InitMono();

        // Load script core and project assembly
        LoadAssemblies();

        // Test class method invocation
        {
            MonoImage* image = mono_assembly_get_image(s_Data->ScriptCoreAssembly);
            MonoClass* klass = mono_class_from_name(image, "", "ScriptCoreTest");

            // Allocate an instance of a class
            MonoObject* class_instance = mono_object_new(s_Data->AppDomain, klass);
            TBO_ENGINE_ASSERT(class_instance, "Could not allocate memory for a class instance!");

            // Call the parameterless constructor
            mono_runtime_object_init(class_instance);

            // Get a reference to the method in the class
            MonoMethod* method = mono_class_get_method_from_name(klass, "PrintHello", 0);
            TBO_ENGINE_ASSERT(method, "Could not find the method!");

            // Call C# method on the object instance
            MonoObject* exception = nullptr;
            mono_runtime_invoke(method, class_instance, nullptr, &exception);


        }

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

    void Script::InitMono()
    {
        // Set path for important C# assemblies
        mono_set_assemblies_path("Mono/lib");

        // Create root domain
        s_Data->RootDomain = mono_jit_init("TBOJITRuntime");
        TBO_ENGINE_ASSERT(s_Data->RootDomain, "Could not initialize mono runtime!");

        // Set main thread as current thread
        mono_thread_set_main(mono_thread_current());

        // Create App Domain
        s_Data->AppDomain = mono_domain_create_appdomain("TBOAppDomain", nullptr);
        TBO_ENGINE_ASSERT(s_Data->AppDomain, "Could not create mono app domain!");

        // Set it as current app domain
        mono_domain_set(s_Data->AppDomain, true);

    }

    void Script::LoadAssemblies()
    {
        s_Data->ScriptCoreAssembly = LoadAssembly("Resources/Scripts/Turbo-ScriptCore.dll");
        s_Data->ScriptCoreAssemblyImage = mono_assembly_get_image(s_Data->ScriptCoreAssembly);

        ReflectAssembly(s_Data->ScriptCoreAssembly);
        // TODO: Project assembly
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

        delete s_Data;
        s_Data = nullptr;
    }

    void Script::ShutdownMono()
    {
        mono_domain_set(s_Data->RootDomain, false);

        mono_domain_unload(s_Data->AppDomain);

        mono_jit_cleanup(s_Data->RootDomain);
    }

}
