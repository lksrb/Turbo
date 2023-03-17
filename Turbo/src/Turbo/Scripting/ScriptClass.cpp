#include "tbopch.h"
#include "ScriptClass.h"

#include "Script.h"

#include <mono/metadata/assembly.h>
#include <mono/metadata/class.h>
#include <mono/metadata/object.h>

namespace Turbo
{
    extern Script::Data* g_Data;

    ScriptClass::ScriptClass(MonoClass* klass, Ref<ScriptClass> base_class)
        : m_MonoClass(klass), m_BaseClass(base_class)
    {
    }

    ScriptClass::ScriptClass(const std::string& namespace_name, const std::string& class_name, Ref<ScriptClass> base_class)
        : m_BaseClass(base_class)
    {
        m_MonoClass = mono_class_from_name(g_Data->ScriptCoreAssemblyImage, namespace_name.c_str(), class_name.c_str());
    }

	ScriptClass::~ScriptClass()
    {
    }

    MonoMethod* ScriptClass::GetMethod(const std::string& method_name, u32 param_count)
    {
        MonoMethod* method = mono_class_get_method_from_name(m_MonoClass, method_name.c_str(), param_count);
        TBO_ENGINE_ASSERT(method);
        return method;
    }

    void ScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* method, void** params)
    {
        MonoObject* exception;
        mono_runtime_invoke(method, instance, params, &exception);

        // TODO: Exception handling
        if (exception)
        {
            MonoString* message = mono_object_to_string(exception, nullptr);
            char* cstring = mono_string_to_utf8(message);
            TBO_FATAL(cstring);
            mono_free(cstring);
        }
    }

}
