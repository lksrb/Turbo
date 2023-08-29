#pragma once

#include "Shader.h"
#include "Texture.h"

namespace Turbo {

    class Material : public RefCounted
    {
    public:
        static Ref<Material> Create(const Ref<Shader>& shader);
        virtual ~Material();

        virtual void Set(std::string_view resourceName, const glm::mat4& matrix) = 0;
        virtual void Set(std::string_view resourceName, const void* data, u64 size) = 0;
        virtual void Set(std::string_view resourceName, const Ref<Texture2D>& texture, u32 index) = 0;
        virtual void Set(std::string_view resourceName, const Ref<TextureCube>& texture) = 0;
    protected:
        Material(const Ref<Shader>& shader);

        Ref<Shader> m_MaterialShader;
    };
}
