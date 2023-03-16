#pragma once

#include "MonoForwards.h"

namespace Turbo
{
    class ScriptClass
    {
    public:
        ScriptClass(const std::string& namespace_name, const std::string& class_name, Ref<ScriptClass> base_class = nullptr);
        ~ScriptClass();

        MonoMethod* GetMethod(const std::string& method_name, u32 param_count);
        void InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);

        MonoClass* GetMonoClass() const { return m_MonoClass; }

        Ref<ScriptClass> GetBaseClass() const { return m_BaseClass; }
    private:
        Ref<ScriptClass> m_BaseClass;
        MonoClass* m_MonoClass;
    };
}
