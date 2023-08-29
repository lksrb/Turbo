#pragma once

#include "Turbo/Core/Buffer.h"
#include "Turbo/Renderer/Material.h"

#include "VulkanBuffer.h"

#include <unordered_map>
#include <vulkan/vulkan.h>

namespace Turbo
{
    class VulkanMaterial : public Material
    {
    public:
        VulkanMaterial(const Ref<Shader>& shader);
        ~VulkanMaterial();

        void Set(std::string_view resourceName, const glm::mat4& matrix) override;
        void Set(std::string_view resourceName, const void* data, u64 size) override;
        void Set(std::string_view resourceName, const Ref<Texture2D>& texture, u32 index) override;
        void Set(std::string_view resourceName, const Ref<TextureCube>& texture) override;
    private:
        void UpdateDescriptors();
    private:
        std::unordered_map<std::string, Ref<VulkanBuffer>> m_UniformBufferMap;

        std::unordered_map<std::string, VkWriteDescriptorSet> m_DescriptorWrites;

        std::unordered_map<std::string, Buffer> m_PushConstants;

        std::vector<VkDescriptorImageInfo> m_TextureDescriptorInfos;
    };
}
