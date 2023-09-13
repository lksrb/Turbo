#pragma once

#include "ScriptClass.h"

#include "Turbo/Core/Time.h"

namespace Turbo
{
    class ScriptInstance : public RefCounted
    {
    private:
        static inline u8 s_FieldValueBuffer[16];

        using OnUpdateMethodFP = void(__stdcall*)(MonoObject*, MonoObject**);
        using OnContactFP = void(__stdcall*)(MonoObject*, u64, MonoObject**);
    public:
        ScriptInstance(Ref<ScriptClass> scriptClass, u64 entity);
        ~ScriptInstance();

        void InvokeOnCreate();
        void InvokeOnUpdate();
        
        void InvokeOnCollisionBegin2D(u64 otherEntity);
        void InvokeOnCollisionEnd2D(u64 otherEntity);
        void InvokeOnTriggerBegin2D(u64 otherEntity);
        void InvokeOnTriggerEnd2D(u64 otherEntity);

        void InvokeOnCollisionBegin(u64 other);
        void InvokeOnCollisionEnd(u64 other);
        void InvokeOnTriggerBegin(u64 other);
        void InvokeOnTriggerEnd(u64 other);

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

        MonoObject* GetMonoInstance() const;

        Ref<ScriptClass> GetScriptClass() const { return m_ScriptClass; }
    private:
        bool GetFieldValueInternal(const std::string& name, void* buffer);
        bool SetFieldValueInternal(const std::string& name, const void* value);
    private:
        MonoMethod* m_Constructor = nullptr;
        MonoMethod* m_OnCreateMethod = nullptr;
        MonoMethod* m_OnUpdateMethod = nullptr;

        // TODO: This is getting out of hand

        MonoMethod* m_OnCollision2DBeginMethod = nullptr;
        MonoMethod* m_OnCollision2DEndMethod = nullptr;
        MonoMethod* m_OnTriggerBegin2DMethod = nullptr;
        MonoMethod* m_OnTriggerEnd2DMethod = nullptr;
        
        MonoMethod* m_OnCollisionBeginMethod = nullptr;
        MonoMethod* m_OnCollisionEndMethod = nullptr;
        MonoMethod* m_OnTriggerBeginMethod = nullptr;
        MonoMethod* m_OnTriggerEndMethod = nullptr;

        // Managed thunks FPs
        OnContactFP m_OnCollisionBegin2DFP = nullptr;
        OnContactFP m_OnCollisionEnd2DFP = nullptr;
        OnContactFP m_OnTriggerBegin2DFP = nullptr;
        OnContactFP m_OnTriggerEnd2DFP = nullptr;

        OnContactFP m_OnCollisionBeginFP = nullptr;
        OnContactFP m_OnCollisionEndFP = nullptr;
        OnContactFP m_OnTriggerBeginFP = nullptr;
        OnContactFP m_OnTriggerEndFP = nullptr;
        
        OnUpdateMethodFP m_OnUpdateMethodFP = nullptr;

        MonoGCHandle m_GCHandle = nullptr;
        MonoObject* m_Exception = nullptr;
        u64 m_Entity;
        Ref<ScriptClass> m_ScriptClass;

        friend class ScriptEngine;
    };
}
