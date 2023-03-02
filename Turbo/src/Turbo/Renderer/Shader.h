#pragma once

#include "Turbo/Core/Filepath.h"

namespace Turbo
{
    enum : u32
    {
        ShaderStage_Vertex = 0,
        ShaderStage_Fragment,
        ShaderStage_Max
    };

    using ShaderStage = u32;

    // Should match VkVertexInputAttributeDescription 
    struct alignas(u32) VertexInputAttribute
    {
        u32 Location;
        u32 Binding;
        u32 Format;
        u32 Offset;
    };

    struct UniformBuffer
    {
        u32 Binding;
        u32 Stage;
        u32 Size;
        String32 Name;
    };

    struct TextureSamplerArray
    {
        String32 Name;
        u32 Binding;
        u32 Size = 0;
    };

    struct ShaderBufferLayout
    {
        std::vector<VertexInputAttribute> Descriptions;
        u32 Stride;
    };

    enum class ShaderLanguage : u32
    {
        GLSL = 0,
        HLSL
    };

    class Shader
    {
    public:
        struct Config
        {
            ShaderLanguage Language;
            Filepath ShaderPath;
        };

        struct Resources
        {
            std::vector<UniformBuffer> UniformBuffers;
            TextureSamplerArray TextureSamplerArray;
        };

        static Ref<Shader> Create(const Shader::Config& config);
        virtual ~Shader();

        // TODO: Hot reloading
        virtual void Reload() = 0;
    protected:
        Shader(const Shader::Config& config);

        Shader::Config m_Config;
    };
}
