#include "tbopch.h"
#include "Script.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

#include <mono/metadata/appdomain.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/tabledefs.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/environment.h>

namespace Turbo
{
    struct ScriptEngineData
    {
        MonoDomain* JITDomain = nullptr;
    };

    static ScriptEngineData* s_Data;

    void Script::Init()
    {
        s_Data = new ScriptEngineData;

        // Initialize mono C# virtual machine
        InitMono();

        TBO_ENGINE_INFO("Successfully initialized mono!");
    }

    void Script::InitMono()
    {
        // Set path for important C# assemblies
        mono_set_assemblies_path("Mono/lib");

        s_Data->JITDomain = mono_jit_init("TBOJITRuntime");
        TBO_ENGINE_ASSERT(s_Data->JITDomain, "Could not initialize mono runtime!");
        
        mono_thread_set_main(mono_thread_current());
    }

    void Script::ShutdownMono()
    {
        mono_jit_cleanup(s_Data->JITDomain);
    }

    void Script::Shutdown()
    {
        ShutdownMono();

        delete s_Data;
        s_Data = nullptr;
    }

}
