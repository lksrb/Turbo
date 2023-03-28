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
    public:
        struct Config
        {
            ShaderLanguage Language;
            std::string ShaderPath;
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
