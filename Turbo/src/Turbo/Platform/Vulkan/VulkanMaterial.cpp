#include "tbopch.h"
#include "VulkanMaterial.h"

#include "VulkanContext.h"
#include "VulkanShader.h"
#include "VulkanTexture2D.h"
#include "VulkanTextureCube.h"

#include "Turbo/Renderer/Renderer.h"

namespace Turbo
{
    VulkanMaterial::VulkanMaterial(const Ref<Shader>& shader)
        : Material(shader)
    {
        UpdateDescriptors();
    }

    VulkanMaterial::~VulkanMaterial()
    {
    }

    void VulkanMaterial::Set(std::string_view resourceName, const glm::mat4& matrix)
    {
        Set(resourceName, &matrix, sizeof(glm::mat4));
    }

    void VulkanMaterial::Set(std::string_view resourceName, const void* data, u64 size)
    {
        const auto& resource = m_UniformBufferMap.find(resourceName.data());
        TBO_ENGINE_ASSERT(resource != m_UniformBufferMap.end(), "Resource does not exists!");

        auto buffer = (*resource).second;
        TBO_ENGINE_ASSERT(buffer->Size() <= sizeof(glm::mat4));
        buffer->SetData(data);
    }

    void VulkanMaterial::Set(std::string_view resourceName, const Ref<Texture2D>& texture, u32 index)
    {
        VkDevice device = VulkanContext::Get()->GetDevice();

        // Find specific resource write descriptor
        const auto& resource = m_DescriptorWrites.find(resourceName.data());
        TBO_ENGINE_ASSERT(resource != m_DescriptorWrites.end()); // No such descriptor exists

        // Convert texture2D 
        Ref<VulkanTexture2D> vulkanTexture = texture.As<VulkanTexture2D>();

        // Set texture index with specific write descriptor 
        m_TextureDescriptorInfos[index].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        m_TextureDescriptorInfos[index].imageView = vulkanTexture->GetImage().As<VulkanImage2D>()->GetImageView();
        m_TextureDescriptorInfos[index].sampler = vulkanTexture->GetImage().As<VulkanImage2D>()->GetSampler();

        // FIXME: Move this
        // Update descriptor set
        VkWriteDescriptorSet& writeInfo = resource->second;
        writeInfo.pImageInfo = m_TextureDescriptorInfos.data();
        vkUpdateDescriptorSets(device, 1, &writeInfo, 0, nullptr);
    }

    void VulkanMaterial::Set(std::string_view resourceName, const Ref<TextureCube>& texture)
    {
        VkDevice device = VulkanContext::Get()->GetDevice();

        // Find specific resource write descriptor
        const auto& resource = m_DescriptorWrites.find(resourceName.data());
        TBO_ENGINE_ASSERT(resource != m_DescriptorWrites.end()); // No such descriptor exists

        auto vkTextureCube = texture.As<VulkanTextureCube>();

        auto& descWrite = m_DescriptorWrites[resourceName.data()];
        VkDescriptorImageInfo imageInfo = {};
        imageInfo.sampler = vkTextureCube->GetSampler();
        imageInfo.imageView = vkTextureCube->GetImageView();
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        descWrite.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(device, 1, &descWrite, 0, nullptr);
    }

    void VulkanMaterial::UpdateDescriptors()
    {
        VkDevice device = VulkanContext::Get()->GetDevice();

        // Creating default samplers, because Vulkan does not support empty samplers (only if extension is enabled, and we want to avoid that)
        VkSampler defaultSampler = VK_NULL_HANDLE;
        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        TBO_VK_ASSERT(vkCreateSampler(device, &samplerInfo, nullptr, &defaultSampler));

        Renderer::SubmitResourceFree([device, defaultSampler]()
        {
            vkDestroySampler(device, defaultSampler, nullptr);
        });

        // Writing to Descriptor set
        Ref<VulkanShader> shader = m_MaterialShader.As<VulkanShader>();
        VulkanShader::Resources resources = shader->GetResources();
        {
            // Texture samplers
            if(resources.TextureSamplerArray.Size)
            {
                m_TextureDescriptorInfos.resize(resources.TextureSamplerArray.Size);

                // Samplers
                for (size_t i = 0; i < m_TextureDescriptorInfos.size(); ++i)
                {
                    m_TextureDescriptorInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    m_TextureDescriptorInfos[i].imageView = nullptr;
                    m_TextureDescriptorInfos[i].sampler = defaultSampler;
                }

                VkWriteDescriptorSet descriptorWrite = {};
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = shader->GetDescriptorSet();
                descriptorWrite.dstBinding = resources.TextureSamplerArray.Binding;
                descriptorWrite.dstArrayElement = 0;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrite.descriptorCount = resources.TextureSamplerArray.Size;
                descriptorWrite.pImageInfo = m_TextureDescriptorInfos.data();

                vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);

                m_DescriptorWrites[resources.TextureSamplerArray.Name] = descriptorWrite;
            }

            for (auto& samplerCubeInfo : resources.SamplerCubeInfos)
            {
                VkDescriptorImageInfo imageInfo = {};
                imageInfo.sampler = defaultSampler;
                imageInfo.imageView = nullptr;
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                VkWriteDescriptorSet descriptorWrite = {};
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = shader->GetDescriptorSet();
                descriptorWrite.dstBinding = samplerCubeInfo.Binding;
                descriptorWrite.dstArrayElement = 0;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrite.descriptorCount = 1;
                descriptorWrite.pImageInfo = &imageInfo;

                vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);

                m_DescriptorWrites[samplerCubeInfo.Name] = descriptorWrite;
            }
        }
    }

}
