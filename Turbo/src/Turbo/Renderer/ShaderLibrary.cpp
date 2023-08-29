#include "tbopch.h"
#include "ShaderLibrary.h"
#include "Turbo/Platform/Vulkan/VulkanShader.h"
#include "Turbo/Debug/ScopeTimer.h"

#include <thread>

namespace Turbo
{
    struct ShaderLibraryInternal
    {
        std::unordered_map<std::string_view, Ref<Shader>> LoadedShaders;
    };

    static ShaderLibraryInternal s_ShaderLibrary;

    void ShaderLibrary::Init()
    {
        ScopeTimer timer("ShaderLibrary::Init");

        s_ShaderLibrary.LoadedShaders["Renderer2D_Quad"] = Shader::Create({ ShaderLanguage::GLSL, "Resources/Shaders/Renderer2D_Quad.glsl" });
        s_ShaderLibrary.LoadedShaders["Renderer2D_Circle"] = Shader::Create({ ShaderLanguage::GLSL, "Resources/Shaders/Renderer2D_Circle.glsl" });
        s_ShaderLibrary.LoadedShaders["Renderer2D_Line"] = Shader::Create({ ShaderLanguage::GLSL, "Resources/Shaders/Renderer2D_Line.glsl" });
        s_ShaderLibrary.LoadedShaders["Renderer2D_Text"] = Shader::Create({ ShaderLanguage::GLSL, "Resources/Shaders/Renderer2D_Text.glsl" });
        s_ShaderLibrary.LoadedShaders["StaticMesh"] = Shader::Create({ ShaderLanguage::GLSL, "Resources/Shaders/StaticMesh.glsl" });
        s_ShaderLibrary.LoadedShaders["Skybox"] = Shader::Create({ ShaderLanguage::GLSL, "Resources/Shaders/Skybox.glsl" });

        for (auto& [_, shader] : s_ShaderLibrary.LoadedShaders)
        {
            Ref<VulkanShader> vulkanShader = shader.As<VulkanShader>();
            vulkanShader->CheckIfUpToDate();
        }

        std::vector<std::thread> compileJobs;
        compileJobs.reserve(s_ShaderLibrary.LoadedShaders.size());

        for (const auto& shader : s_ShaderLibrary.LoadedShaders)
        {
           compileJobs.emplace_back([shader]()
            {
                Ref<VulkanShader> vulkanShader = shader.second.As<VulkanShader>();
                vulkanShader->ReadAndPreprocess();
                vulkanShader->CompileOrGetCompiledShaders();
                vulkanShader->CreateModules();
            });
        }

        // Wait for all threads
        for (auto& job : compileJobs)
        {
            job.join();
        }

        // NOTE: We can do this also in parellel to other jobs but because of the printing VulkanShader::Reflect, the console output is screwed up
        // TODO: Make this parallel 
        for (auto& [_, shader] : s_ShaderLibrary.LoadedShaders)
        {
            Ref<VulkanShader> vulkanShader = shader.As<VulkanShader>();
            vulkanShader->Reflect();
            vulkanShader->GenerateDescriptors();
        }
    }

    void ShaderLibrary::Shutdown()
    {
        s_ShaderLibrary.LoadedShaders.clear();
    }

    Ref<Shader> ShaderLibrary::Get(std::string_view shaderName)
    {
        return s_ShaderLibrary.LoadedShaders.at(shaderName);
    }

}
