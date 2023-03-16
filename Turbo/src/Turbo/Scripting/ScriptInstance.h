#pragma once

#include "ScriptClass.h"

#include "Turbo/Core/Time.h"

namespace Turbo
{
    class ScriptInstance
    {
    public:
        ScriptInstance(Ref<ScriptClass> script_class, u64 entity);
        ~ScriptInstance();

        void InvokeOnStart();
        void InvokeOnUpdate(FTime ts);

        MonoObject* GetInstance() const { return m_Instance; }
    private:
        MonoMethod* m_OnStartMethod = nullptr;
        MonoMethod* m_OnUpdateMethod = nullptr;
        MonoMethod* m_Constructor = nullptr;;

        MonoObject* m_Instance = nullptr;

        u64 m_Entity;
        Ref<ScriptClass> m_ScriptClass;
    };
}
