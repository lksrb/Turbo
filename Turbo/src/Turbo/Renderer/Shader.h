#pragma once

#include <string>

namespace Turbo
{
    // Used to more clear iteration over shaders
    enum : u32
    {
        ShaderStage_Vertex = 0,
        ShaderStage_Fragment,

        ShaderStage_Count
    };
    using ShaderStage = u32;

    enum class ShaderLanguage : u32
    {
        GLSL = 0,
        HLSL
    };

    class Shader : public RefCounted
    {  
    public:
        struct Config
        {
            ShaderLanguage Language;
            std::string ShaderPath;
        };

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

        struct SamplerCubeInfo
        {
            std::string Name;
            u32 Binding;
            u32 DescriptorSet;
        };

        struct PushConstantRange
        {
            u32 Offset;
            u32 Size;
            ShaderStage Stage;
        };

        struct Resources
        {
            std::vector<UniformBufferInfo> UniformBuffers;
            TextureSamplerArrayInfo TextureSamplerArray;
            std::vector<SamplerCubeInfo> SamplerCubeInfos;
            std::vector<PushConstantRange> PushConstantRanges;
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
