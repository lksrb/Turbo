#pragma once

#include "Turbo/Renderer/GraphicsPipeline.h"

#include <vulkan/vulkan.h>

namespace Turbo
{
    class VulkanGraphicsPipeline : public GraphicsPipeline
    {
    public:
        VulkanGraphicsPipeline(const GraphicsPipeline::Config& config);
        ~VulkanGraphicsPipeline();

        VkPipeline GetPipelineHandle() const { return m_Pipeline; }
        VkPipelineLayout GetPipelineLayoutHandle() const { return m_PipelineLayout; }

        void Invalidate() override;
    private:
        VkPipeline m_Pipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
    };
}
