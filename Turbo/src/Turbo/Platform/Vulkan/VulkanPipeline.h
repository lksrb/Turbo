#pragma once

#include "Turbo/Renderer/Pipeline.h"

#include <vulkan/vulkan.h>

namespace Turbo
{
    class VulkanPipeline : public Pipeline
    {
    public:
        VulkanPipeline(const Pipeline::Config& config);
        ~VulkanPipeline();

        VkPipeline GetPipelineHandle() const { return m_Pipeline; }
        VkPipelineLayout GetPipelineLayoutHandle() const { return m_PipelineLayout; }

        void Invalidate() override;
    private:
        VkPipeline m_Pipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
    };
}
