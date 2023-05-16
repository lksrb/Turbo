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
    ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, u64 entity)
        : m_ScriptClass(scriptClass), m_Entity(entity)
    {
        // NOTE: Kinda weird
        Ref<ScriptClass> baseClass = m_ScriptClass->GetBaseClass() ? m_ScriptClass->GetBaseClass() : Script::GetEntityBaseClass();

        m_Constructor = baseClass->GetMethod(".ctor", 1);

        MonoObject* instance = mono_object_new(Script::GetAppDomain(), m_ScriptClass->GetMonoClass());

        // Call default constructor
        mono_runtime_object_init(instance);

        m_GCHandle = mono_gchandle_new_v2(instance, false);

        // TODO: Maybe there is a better solution?
        // Prevert crash from client not specifing OnCreate or OnUpdate methods
        {
            MonoMethod* onCreate = scriptClass->GetMethod("OnCreate", 0);
            MonoMethod* onUpdate = scriptClass->GetMethod("OnUpdate", 0);
            // Gets overriden methods
            m_OnCreateMethod = onCreate != nullptr ? onCreate : baseClass->GetMethod("OnCreate", 0);
            m_OnUpdateMethod = onUpdate != nullptr ? onUpdate : baseClass->GetMethod("OnUpdate", 0);
        }

        // Physics2D
        m_OnCollision2DBeginMethod = baseClass->GetMethod("OnCollisionBegin2D_Internal", 1);
        m_OnCollision2DEndMethod = baseClass->GetMethod("OnCollisionEnd2D_Internal", 1);
        m_OnTriggerBegin2DMethod = baseClass->GetMethod("OnTriggerBegin2D_Internal", 1);
        m_OnTriggerEnd2DMethod = baseClass->GetMethod("OnTriggerEnd2D_Internal", 1);

        // Invoke base class constructor and assign it the entity id
        void* params = &m_Entity;
        m_ScriptClass->InvokeMethod(instance, m_Constructor, nullptr, &params);

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
        mono_gchandle_free_v2(m_GCHandle);
    }

    void ScriptInstance::InvokeOnCreate()
    {
        if (m_Exception)
        {
            MonoString* message = mono_object_to_string(m_Exception, nullptr);
            char* cstring = mono_string_to_utf8(message);
            TBO_CONSOLE_FATAL(cstring);
            mono_free(cstring);

            return;
        }

        m_ScriptClass->InvokeMethod(GetMonoInstance(), m_OnCreateMethod, &m_Exception);
    }

    void ScriptInstance::InvokeOnUpdate()
    {
        if (m_Exception)
        {
            MonoString* message = mono_object_to_string(m_Exception, nullptr);
            char* cstring = mono_string_to_utf8(message);
            TBO_CONSOLE_FATAL(cstring);
            mono_free(cstring);

            return;
        }

#if TBO_USE_MANAGED_THUNKS
        // NOTE: When an exception is thrown while the debugger is attached,
        // with managed thunks, debugger does not tell client that an exception has been thrown other than is prints into console.
        // With mono_runtime_invoke, this works
        // TODO: Find out how to show exception to a client in his opened editor

        m_OnUpdateMethodFP(GetMonoInstance(), &m_Exception);

#else
        m_ScriptClass->InvokeMethod(GetMonoInstance(), m_OnUpdateMethod, &m_Exception);
#endif
    }

    void ScriptInstance::InvokeOnCollisionBegin2D(u64 otherEntity)
    {
        if (m_Exception)
        {
            MonoString* message = mono_object_to_string(m_Exception, nullptr);
            char* cstring = mono_string_to_utf8(message);
            TBO_CONSOLE_FATAL(cstring);
            mono_free(cstring);

            return;
        }

#if TBO_USE_MANAGED_THUNKS
        m_OnCollisionBegin2DFP(GetMonoInstance(), otherEntity, &m_Exception);

#else
        void* params = &otherEntity;
        m_ScriptClass->InvokeMethod(GetMonoInstance(), m_OnCollision2DBeginMethod, &m_Exception, &params);
#endif
    }

    void ScriptInstance::InvokeOnCollisionEnd2D(u64 otherEntity)
    {
        if (m_Exception)
        {
            MonoString* message = mono_object_to_string(m_Exception, nullptr);
            char* cstring = mono_string_to_utf8(message);
            TBO_CONSOLE_FATAL(cstring);
            mono_free(cstring);
            return;
        }

#if TBO_USE_MANAGED_THUNKS
        m_OnCollisionEnd2DFP(GetMonoInstance(), otherEntity, &m_Exception);
#else
        void* params = &otherEntity;
        m_ScriptClass->InvokeMethod(GetMonoInstance(), m_OnCollision2DEndMethod, &m_Exception, &params);
#endif
    }

    void ScriptInstance::InvokeOnTriggerBegin2D(u64 otherEntity)
    {
        if (m_Exception)
        {
            MonoString* message = mono_object_to_string(m_Exception, nullptr);
            char* cstring = mono_string_to_utf8(message);
            TBO_CONSOLE_FATAL(cstring);
            mono_free(cstring);
            return;
        }

#if TBO_USE_MANAGED_THUNKS
        m_OnTriggerBegin2DFP(GetMonoInstance(), otherEntity, &m_Exception);
#else
        void* params = &otherEntity;
        m_ScriptClass->InvokeMethod(GetMonoInstance(), m_OnTriggerBegin2DMethod, &m_Exception, &params);
#endif
    }

    void ScriptInstance::InvokeOnTriggerEnd2D(u64 otherEntity)
    {
        if (m_Exception)
        {
            MonoString* message = mono_object_to_string(m_Exception, nullptr);
            char* cstring = mono_string_to_utf8(message);
            TBO_CONSOLE_FATAL(cstring);
            mono_free(cstring);
            return;
        }

#if TBO_USE_MANAGED_THUNKS
        m_OnTriggerEnd2DFP(GetMonoInstance(), otherEntity, &m_Exception);
#else
        void* params = &otherEntity;
        m_ScriptClass->InvokeMethod(GetMonoInstance(), m_OnTriggerEnd2DMethod, &m_Exception, &params);
#endif
    }

    MonoObject* ScriptInstance::GetMonoInstance() const
    {
        return mono_gchandle_get_target_v2(m_GCHandle);
    }

    bool ScriptInstance::GetFieldValueInternal(const std::string& name, void* buffer)
    {
        const auto& fields = m_ScriptClass->GetFields();
        auto& it = fields.find(name);
        if (it == fields.end())
            return false;

        const ScriptField& field = it->second;
        mono_field_get_value(GetMonoInstance(), field.MonoField, buffer);

        return true;
    }

    bool ScriptInstance::SetFieldValueInternal(const std::string& name, const void* value)
    {
        const auto& fields = m_ScriptClass->GetFields();
        auto& it = fields.find(name);
        if (it == fields.end())
            return false;

        const ScriptField& field = it->second;
        mono_field_set_value(GetMonoInstance(), field.MonoField, (void*)value);

        return true;

    }

}
