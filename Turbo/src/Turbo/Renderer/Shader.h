#pragma once

namespace Turbo
{
    enum : u32
    {
        ShaderStage_Vertex = 0,
        ShaderStage_Fragment,
        ShaderStage_Max
    };

    using ShaderStage = u32;

    enum class ShaderLanguage : u32
    {
        GLSL = 0,
        HLSL
    };

    class Shader
    {
    protected:
        struct UniformBufferInfo
        {
            u32 Binding;
            u32 Stage;
            u32 Size;
            std::string Name;
        };

        struct TextureSamplerArrayInfo
        {
            std::string Name;
            u32 Binding;
            u32 Size = 0;
        };

        // Should match VkVertexInputAttributeDescription 
        struct VertexInputAttribute
        {
            u32 Location;
            u32 Binding;
            u32 Format;
            u32 Offset;
        };

        struct ShaderBufferLayout
        {
            std::vector<VertexInputAttribute> Descriptions;
            u32 Stride;
        };

    public:
        struct Config
        {
            ShaderLanguage Language;
            std::string ShaderPath;
        };

        struct Resources
        {
            std::vector<UniformBufferInfo> UniformBuffers;
            TextureSamplerArrayInfo TextureSamplerArray;
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
