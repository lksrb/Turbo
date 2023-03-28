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

        struct ShaderBufferLayout
        {
            std::vector<VkVertexInputAttributeDescription> Descriptions;
            u32 Stride;
        };

        struct Resources
        {
            std::vector<UniformBufferInfo> UniformBuffers;
            TextureSamplerArrayInfo TextureSamplerArray;
        };

        VulkanShader(const Shader::Config& config);
        ~VulkanShader();

        VkShaderModule GetModule(ShaderStage shaderStage) { TBO_ENGINE_ASSERT(shaderStage < ShaderStage_Max); return m_ShaderModules[shaderStage]; }

        const ShaderBufferLayout& GetLayout() const { return m_Layout; }
        const Resources& GetResources() const { return m_Resources; }

        VkDescriptorSetLayout GetDescriptorSetLayout() const { return m_DescriptorSetLayout; }
        VkDescriptorSet GetDescriptorSet() const { return m_DescriptorSet; }

        void Reload() override;
    private:
        void ReadAndPreprocess();
        void CheckIfUpToDate();
        void CompileOrGetCompiledShaders();
        void CompileShader(ShaderStage shaderStage);
        void Reflect();
        void ReflectStage(ShaderStage shaderStage);
        void CreateModules();
        void GenerateDescriptors();
    private:
        VkDescriptorSetLayout m_DescriptorSetLayout;
        VkDescriptorPool m_DescriptorPool;
        VkDescriptorSet m_DescriptorSet;

        ShaderBufferLayout m_Layout;
        bool m_Compile{ false };
        VkShaderModule m_ShaderModules[ShaderStage_Max];
        std::string m_ShaderSources[ShaderStage_Max];
        std::vector<u32> m_CompiledShaders[ShaderStage_Max];
        
        Resources m_Resources;
    };

}
