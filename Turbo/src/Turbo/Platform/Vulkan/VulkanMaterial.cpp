#include "tbopch.h"
#include "VulkanMaterial.h"

#include "Turbo/Renderer/RendererContext.h"

#include "VulkanShader.h"
#include "VulkanTexture2D.h"

namespace Turbo
{
    VulkanMaterial::VulkanMaterial(const Material::Config& config)
        : Material(config)
    {
        CreateWriteDescriptors();
    }

    VulkanMaterial::~VulkanMaterial()
    {
        for (auto& [name, uniformBuffer] : m_UniformBufferMap)
        {
            delete uniformBuffer;
        }
    }

    void VulkanMaterial::Set(const FString32& resourceName, const glm::mat4& matrix)
    {
        Set(resourceName, &matrix, sizeof(glm::mat4));
    }

    void VulkanMaterial::Set(const FString32& resourceName, const void* data, size_t size)
    {
        auto& resource = m_UniformBufferMap.find(resourceName.c_str());
        TBO_ENGINE_ASSERT(resource != m_UniformBufferMap.end(), "Resource does not exists!");

        auto buffer = (*resource).second;
        TBO_ENGINE_ASSERT(buffer->GetSize() <= sizeof(glm::mat4));
        buffer->SetData(data);
    }

    void VulkanMaterial::Set(const FString32& resourceName, const Ptr<Texture2D>& texture, u32 index)
    {
        VkDevice device = RendererContext::GetDevice();

        // Find specific resource write descriptor
        auto& resource = m_DescriptorWrites.find(resourceName.c_str());
        TBO_ENGINE_ASSERT(resource != m_DescriptorWrites.end()); // No such descriptor exists

        // Convert texture2D 
        const VulkanTexture2D* vulkanTexture = texture.As<VulkanTexture2D>();

        // Set texture index with specific write descriptor 
        m_TextureDescriptorInfos[index].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        m_TextureDescriptorInfos[index].imageView = vulkanTexture->GetImage2D().As<VulkanImage2D>()->GetImageView();
        m_TextureDescriptorInfos[index].sampler = vulkanTexture->GetSampler();

        // FIXME: Move this
        // Update descriptor set
        auto& descriptorWrite = resource->second;
        descriptorWrite.pImageInfo = m_TextureDescriptorInfos.data();
        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }

    void VulkanMaterial::Update()
    {
        // Invalidate descriptor sets
    }

    void VulkanMaterial::CreateWriteDescriptors()
    {
        VkDevice device = RendererContext::GetDevice();

        // Creating default samplers, because Vulkan does not support empty samplers (only if extension is enabled, which is a bit problematic)
        VkSampler defaultSampler = VK_NULL_HANDLE;
        VkSamplerCreateInfo samplerInfo{};
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

        RendererContext::GetResourceQueue().Submit(SAMPLER, [device, defaultSampler]()
        {
            vkDestroySampler(device, defaultSampler, nullptr);
        });

        // Writing to Descriptor set
        VulkanShader* shader = m_Config.MaterialShader.As<VulkanShader>();
        Shader::Resources resources = shader->GetResources();
        {
            // Uniform buffers
            for (auto& uniformBuffer : resources.UniformBuffers)
            {
                // Create new vulkan buffer for resource
                RendererBuffer::Config bufferConfig = {};
                bufferConfig.Size = uniformBuffer.Size;
                bufferConfig.UsageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
                bufferConfig.MemoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
                m_UniformBufferMap[uniformBuffer.Name.c_str()] = new VulkanBuffer(bufferConfig);

                VkDescriptorBufferInfo bufferInfo{};
                bufferInfo.buffer = m_UniformBufferMap[uniformBuffer.Name.c_str()]->GetBuffer();
                bufferInfo.offset = 0;
                bufferInfo.range = uniformBuffer.Size;

                VkWriteDescriptorSet descriptorWrite{};
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = shader->GetDescriptorSet();
                descriptorWrite.dstBinding = uniformBuffer.Binding;
                descriptorWrite.dstArrayElement = 0;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorWrite.descriptorCount = 1;
                descriptorWrite.pBufferInfo = &bufferInfo;

                vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
            }

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

                VkWriteDescriptorSet descriptorWrite{};
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = shader->GetDescriptorSet();
                descriptorWrite.dstBinding = resources.TextureSamplerArray.Binding;
                descriptorWrite.dstArrayElement = 0;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrite.descriptorCount = resources.TextureSamplerArray.Size;
                descriptorWrite.pImageInfo = m_TextureDescriptorInfos.data();

                vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);

                m_DescriptorWrites[resources.TextureSamplerArray.Name.c_str()] = descriptorWrite;
            }
        }
    }

}
