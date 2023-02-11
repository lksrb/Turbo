#include "tbopch.h"
#include "Shader.h"

#include "Turbo/Platform/Vulkan/VulkanShader.h"

namespace Turbo
{
    Shader::Shader(const Shader::Config& config)
        : m_Config(config)
    {
    }

    Shader::~Shader()
    {
    }

    Ref<Shader> Shader::Create(const Shader::Config& config)
    {
        TBO_ENGINE_ASSERT(config.Language == ShaderLanguage::GLSL, "HLSL not supported yet!");
        return Ref<VulkanShader>::Create(config);
    }
}
