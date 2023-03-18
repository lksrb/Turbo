#pragma once

#include "MonoForwards.h"

#include "ScriptField.h"

namespace Turbo
{
    class ScriptClass
    {
    public:
        using ScriptFields = std::unordered_map<std::string, ScriptField>;

        ScriptClass(MonoClass* klass, Ref<ScriptClass> base_class = nullptr);
        ScriptClass(const std::string& namespace_name, const std::string& class_name, Ref<ScriptClass> base_class = nullptr);
        ~ScriptClass();

        MonoMethod* GetMethod(const std::string& method_name, u32 param_count);
        void InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);

        MonoClass* GetMonoClass() const { return m_MonoClass; }

        const ScriptFields& GetFields() const { return m_ScriptFields; }

        Ref<ScriptClass> GetBaseClass() const { return m_BaseClass; }
    private:
        ScriptFields m_ScriptFields;

        Ref<ScriptClass> m_BaseClass;
        MonoClass* m_MonoClass;

        friend class Script;
    };
}
