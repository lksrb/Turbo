#include "tbopch.h"
#include "ScriptInstance.h"

#include "Script.h"

#include "Turbo/Benchmark/ScopeTimer.h"

#include <mono/metadata/assembly.h>
#include <mono/metadata/class.h>
#include <mono/metadata/object.h>

#define TBO_USE_MANAGED_THUNKS 1

namespace Turbo
{
    extern Script::Data* g_Data;

    ScriptInstance::ScriptInstance(Ref<ScriptClass> script_class, u64 entity)
        : m_ScriptClass(script_class), m_Entity(entity)
    {
        m_Constructor = m_ScriptClass->GetBaseClass()->GetMethod(".ctor", 1);
        m_OnStartMethod = m_ScriptClass->GetMethod("OnStart", 0);
        m_OnUpdateMethod = m_ScriptClass->GetMethod("OnUpdate", 1);

        // Allocate new instance
        m_Instance = mono_object_new(g_Data->AppDomain, m_ScriptClass->GetMonoClass());
        mono_runtime_object_init(m_Instance);

        // Invoke base class constructor and assign it the entity id
        void* params = &m_Entity;
        m_ScriptClass->InvokeMethod(m_Instance, m_Constructor, &params);

        // Managed thunks
        m_OnUpdateMethodFP = (OnUpdateMethod)mono_method_get_unmanaged_thunk(m_OnUpdateMethod);
    }

    ScriptInstance::~ScriptInstance()
    {
    }

    void ScriptInstance::InvokeOnStart()
    {
        m_ScriptClass->InvokeMethod(m_Instance, m_OnStartMethod);
    }

    void ScriptInstance::InvokeOnUpdate(FTime ts)
    {
#if TBO_USE_MANAGED_THUNKS
        MonoException* e;
        m_OnUpdateMethodFP(m_Instance, ts, &e);
        TBO_ENGINE_ASSERT(!e);
#else
        void* params = &ts;
        m_ScriptClass->InvokeMethod(m_Instance, m_OnUpdateMethod, &params);
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
