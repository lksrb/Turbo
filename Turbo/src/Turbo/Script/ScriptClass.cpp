#include "tbopch.h"
#include "ScriptClass.h"

#include "Script.h"

#include <mono/metadata/assembly.h>
#include <mono/metadata/class.h>
#include <mono/metadata/object.h>

namespace Turbo
{
    ScriptClass::ScriptClass(MonoClass* klass, Ref<ScriptClass> base_class)
        : m_MonoClass(klass), m_BaseClass(base_class)
    {
    }

    ScriptClass::ScriptClass(const std::string& nameSpace, const std::string& className, Ref<ScriptClass> baseClas)
        : m_BaseClass(baseClas)
    {
        m_MonoClass = mono_class_from_name(Script::GetCoreAssemblyImage(), nameSpace.c_str(), className.c_str());
        m_Size = (u64)mono_class_instance_size(m_MonoClass);
    }

	ScriptClass::~ScriptClass()
    {
    }

    MonoMethod* ScriptClass::GetMethod(const std::string& methodName, u32 paramCount)
    {
        MonoMethod* method = mono_class_get_method_from_name(m_MonoClass, methodName.c_str(), paramCount);
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
