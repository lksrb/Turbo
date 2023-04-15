#include "tbopch.h"
#include "ScriptInstance.h"

#include "Script.h"

#include "Turbo/Debug/ScopeTimer.h"

#include <mono/metadata/assembly.h>
#include <mono/metadata/class.h>
#include <mono/metadata/object.h>

#define TBO_USE_MANAGED_THUNKS 1

namespace Turbo
{
    extern Script::Data* g_Data;

    ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, u64 entity)
        : m_ScriptClass(scriptClass), m_Entity(entity)
    {
        m_Constructor = m_ScriptClass->GetBaseClass()->GetMethod(".ctor", 1);
       
        m_Instance = mono_object_new(g_Data->AppDomain, m_ScriptClass->GetMonoClass());
        mono_runtime_object_init(m_Instance);

        // Gets overriden methods
        m_OnCreateMethod = mono_object_get_virtual_method(m_Instance, m_ScriptClass->GetBaseClass()->GetMethod("OnCreate", 0));
        m_OnUpdateMethod = mono_object_get_virtual_method(m_Instance, m_ScriptClass->GetBaseClass()->GetMethod("OnUpdate", 1));

        // Physics2D
        m_OnCollision2DBeginMethod = m_ScriptClass->GetBaseClass()->GetMethod("OnCollisionBegin2D_Internal", 1);
        m_OnCollision2DEndMethod = m_ScriptClass->GetBaseClass()->GetMethod("OnCollisionEnd2D_Internal", 1);
        m_OnTriggerBegin2DMethod = m_ScriptClass->GetBaseClass()->GetMethod("OnTriggerBegin2D_Internal", 1);
        m_OnTriggerEnd2DMethod = m_ScriptClass->GetBaseClass()->GetMethod("OnTriggerEnd2D_Internal", 1);

        // Invoke base class constructor and assign it the entity id
        void* params = &m_Entity;
        m_ScriptClass->InvokeMethod(m_Instance, m_Constructor, &params);

#if TBO_USE_MANAGED_THUNKS
        // Managed thunks
        m_OnUpdateMethodFP = (OnUpdateMethodFP)mono_method_get_unmanaged_thunk(m_OnUpdateMethod);
        m_OnCollisionBegin2DFP = (OnCollision2DFP)mono_method_get_unmanaged_thunk(m_OnCollision2DBeginMethod);
        m_OnCollisionEnd2DFP = (OnCollision2DFP)mono_method_get_unmanaged_thunk(m_OnCollision2DEndMethod);
        m_OnTriggerBegin2DFP = (OnCollision2DFP)mono_method_get_unmanaged_thunk(m_OnTriggerBegin2DMethod);
        m_OnTriggerEnd2DFP = (OnCollision2DFP)mono_method_get_unmanaged_thunk(m_OnTriggerEnd2DMethod);
#endif
    }

    ScriptInstance::~ScriptInstance()
    {
    }

    void ScriptInstance::InvokeOnCreate()
    {
        m_ScriptClass->InvokeMethod(m_Instance, m_OnCreateMethod);
    }

    void ScriptInstance::InvokeOnUpdate(FTime ts)
    {
#if TBO_USE_MANAGED_THUNKS
        // NOTE: When an exception is thrown while the debugger is attached,
        // with managed thunks, debugger does not tell client that an exception has been thrown other than is prints into console.
        // With mono_runtime_invoke, this works
        // TODO: Find out how to show exception to a client in his opened editor

        MonoObject* exception;
        m_OnUpdateMethodFP(m_Instance, ts, &exception);

        if (exception)
        {
            MonoString* message = mono_object_to_string(exception, nullptr);
            char* cstring = mono_string_to_utf8(message);
            TBO_FATAL(cstring);
            mono_free(cstring);
        }
#else
        void* params = &ts;
        m_ScriptClass->InvokeMethod(m_Instance, m_OnUpdateMethod, &params);
#endif
    }

    void ScriptInstance::InvokeOnCollisionBegin2D(u64 otherEntity)
    {
#if TBO_USE_MANAGED_THUNKS
        MonoObject* exception;
        m_OnCollisionBegin2DFP(m_Instance, otherEntity, &exception);

        if (exception)
        {
            MonoString* message = mono_object_to_string(exception, nullptr);
            char* cstring = mono_string_to_utf8(message);
            TBO_FATAL(cstring);
            mono_free(cstring);
        }
#else
        void* params = &ts;
        m_ScriptClass->InvokeMethod(m_Instance, m_OnCollisionBegin2DFP, &params);
#endif
    }

    void ScriptInstance::InvokeOnCollisionEnd2D(u64 otherEntity)
    {
#if TBO_USE_MANAGED_THUNKS
        MonoObject* exception;
        m_OnCollisionEnd2DFP(m_Instance, otherEntity, &exception);

        if (exception)
        {
            MonoString* message = mono_object_to_string(exception, nullptr);
            char* cstring = mono_string_to_utf8(message);
            TBO_FATAL(cstring);
            mono_free(cstring);
        }
#else
        void* params = &ts;
        m_ScriptClass->InvokeMethod(m_Instance, m_OnCollisionEnd2DFP, &params);
#endif
    }

    void ScriptInstance::InvokeOnTriggerBegin2D(u64 otherEntity)
    {
#if TBO_USE_MANAGED_THUNKS
        MonoObject* exception;
        m_OnTriggerBegin2DFP(m_Instance, otherEntity, &exception);

        if (exception)
        {
            MonoString* message = mono_object_to_string(exception, nullptr);
            char* cstring = mono_string_to_utf8(message);
            TBO_FATAL(cstring);
            mono_free(cstring);
        }
#else
        void* params = &ts;
        m_ScriptClass->InvokeMethod(m_Instance, m_OnCollisionEnd2DFP, &params);
#endif
    }

    void ScriptInstance::InvokeOnTriggerEnd2D(u64 otherEntity)
    {
#if TBO_USE_MANAGED_THUNKS
        MonoObject* exception;
        m_OnTriggerEnd2DFP(m_Instance, otherEntity, &exception);

        if (exception)
        {
            MonoString* message = mono_object_to_string(exception, nullptr);
            char* cstring = mono_string_to_utf8(message);
            TBO_FATAL(cstring);
            mono_free(cstring);
        }
#else
        void* params = &ts;
        m_ScriptClass->InvokeMethod(m_Instance, m_OnCollisionEnd2DFP, &params);
#endif
    }

    bool ScriptInstance::GetFieldValueInternal(const std::string& name, void* buffer)
    {
        const auto& fields = m_ScriptClass->GetFields();
        auto& it = fields.find(name);
        if (it == fields.end())
            return false;

        const ScriptField& field = it->second;
        mono_field_get_value(m_Instance, field.MonoField, buffer);

        return true;
    }

    bool ScriptInstance::SetFieldValueInternal(const std::string& name, const void* value)
    {
        const auto& fields = m_ScriptClass->GetFields();
        auto& it = fields.find(name);
        if (it == fields.end())
            return false;

        const ScriptField& field = it->second;
        mono_field_set_value(m_Instance, field.MonoField, (void*)value);

        return true;

    }

}
