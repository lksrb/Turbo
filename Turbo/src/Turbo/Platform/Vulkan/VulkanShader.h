#pragma once

#include "Turbo/Core/PrimitiveTypes.h"

#include "Turbo/Renderer/Shader.h"

#include <vulkan/vulkan.h>

#include <unordered_map>
#include <string>
#include <vector>

namespace Turbo
{
    class VulkanShader : public Shader
    {
    public:
        struct ShaderBufferLayout
        {
            std::vector<VkVertexInputAttributeDescription> Descriptions;
            u32 Stride;
        };

        VulkanShader(const Shader::Config& config);
        ~VulkanShader();

        VkShaderModule GetModule(ShaderStage shaderStage) { TBO_ENGINE_ASSERT(shaderStage < ShaderStage_Count); return m_ShaderModules[shaderStage]; }

        const Resources& GetResources() const { return m_Resources; }

        VkDescriptorSetLayout GetDescriptorSetLayout() const { return m_DescriptorSetLayout; }
        VkDescriptorSet GetDescriptorSet() const { return m_DescriptorSet; }

        void Reload() override;
    private:
        void ReadAndPreprocess();
        void CheckIfUpToDate();
        void CompileOrGetCompiledShaders();
        void CompileShader(ShaderStage shaderStage);
        void CompileShaders();
        void Reflect();
        void ReflectStage(ShaderStage shaderStage);
        void CreateModules();
        void GenerateDescriptors();
        std::filesystem::path GetShaderCachePath(ShaderStage stage);
    private:
        VkDescriptorSetLayout m_DescriptorSetLayout;
        VkDescriptorPool m_DescriptorPool;
        VkDescriptorSet m_DescriptorSet;

        ShaderBufferLayout m_Layout;
        bool m_Compile{ false };
        VkShaderModule m_ShaderModules[ShaderStage_Count];
        std::string m_ShaderSources[ShaderStage_Count];
        std::vector<u32> m_CompiledShaders[ShaderStage_Count];
        
        Resources m_Resources;

        friend class ShaderLibrary;
    };

}
