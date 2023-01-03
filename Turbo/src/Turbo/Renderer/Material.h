#pragma once

#include "Turbo/Renderer/Shader.h"
#include "Turbo/Renderer/Texture2D.h"
#include "Turbo/Renderer/RendererBuffer.h"

#include <glm/glm.hpp>

namespace Turbo
{
    class Material
    {
    public:
        struct Config
        {
            Ptr<Shader> MaterialShader;
        };

        static Material* Create(const Material::Config& config);
        virtual ~Material();

        virtual void Set(const FString32& resourceName, const glm::mat4& matrix) = 0;
        virtual void Set(const FString32& resourceName, const void* data, size_t size) = 0;
        virtual void Set(const FString32& resourceName, const Ptr<Texture2D>& texture, u32 index) = 0;

        virtual void Update() = 0;
    protected:
        Material(const Material::Config& config);

        Material::Config m_Config;
    };
}
