#include "tbopch.h"
#include "ScriptInstance.h"

#include "Script.h"

#include <mono/metadata/assembly.h>
#include <mono/metadata/class.h>
#include <mono/metadata/object.h>

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
        void* params = &ts;
        m_ScriptClass->InvokeMethod(m_Instance, m_OnUpdateMethod, &params);
    }

}
