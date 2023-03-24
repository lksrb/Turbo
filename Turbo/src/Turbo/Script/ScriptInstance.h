#pragma once

#include "ScriptClass.h"

#include "Turbo/Core/Time.h"

namespace Turbo
{
    class ScriptInstance
    {
    private:
        using OnUpdateMethod = void(__stdcall*)(MonoObject*, f32, MonoObject**);
        static inline u8 s_FieldValueBuffer[16];
    public:
        ScriptInstance(Ref<ScriptClass> script_class, u64 entity);
        ~ScriptInstance();

        void InvokeOnStart();
        void InvokeOnUpdate(FTime ts);

        template<typename T>
        T GetFieldValue(const std::string& name)
        {
            static_assert(sizeof(T) <= 16, "Type too large!");

            bool success = GetFieldValueInternal(name, s_FieldValueBuffer);

            if (success == false)
                return T();

            return *(T*)s_FieldValueBuffer;
        }

        template<typename T>
        void SetFieldValue(const std::string& name, T* value)
        {
            static_assert(sizeof(T) <= 16, "Type too large!");

            SetFieldValueInternal(name, value);
        }


        MonoObject* GetInstance() const { return m_Instance; }

        Ref<ScriptClass> GetScriptClass() const { return m_ScriptClass; }
    private:
        bool GetFieldValueInternal(const std::string& name, void* buffer);
        bool SetFieldValueInternal(const std::string& name, const void* value);
    private:
        MonoMethod* m_OnStartMethod = nullptr;
        OnUpdateMethod m_OnUpdateMethodFP = nullptr; // Managed thunks
        MonoMethod* m_OnUpdateMethod = nullptr;
        MonoMethod* m_Constructor = nullptr;;

        MonoObject* m_Instance = nullptr;

        u64 m_Entity;
        Ref<ScriptClass> m_ScriptClass;

        friend class Script;
    };
}
