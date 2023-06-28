#pragma once

#include "Turbo/Renderer/Material.h"

#include "VulkanBuffer.h"

#include <unordered_map>
#include <vulkan/vulkan.h>

namespace Turbo
{
    class VulkanMaterial : public Material
    {
    public:
        VulkanMaterial(const Material::Config& config);
        ~VulkanMaterial();

        void Set(const std::string& resourceName, const glm::mat4& matrix) override;
        void Set(const std::string& resourceName, const void* data, size_t size) override;
        void Set(const std::string& resourceName, const Ref<Texture2D>& texture, u32 index) override;

        void Update() override;
    private:
        void UpdateDescriptors();
    private:
        std::unordered_map<std::string, Ref<VulkanBuffer>> m_UniformBufferMap;
        std::unordered_map<std::string, std::vector<VkDescriptorImageInfo>> m_TextureDescriptorMap;

        std::unordered_map<std::string, VkWriteDescriptorSet> m_DescriptorWrites;

        std::vector<VkDescriptorImageInfo> m_TextureDescriptorInfos;
        std::vector<Ref<Texture2D>> m_Textures;
    };
}
