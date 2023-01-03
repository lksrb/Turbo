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

        VkPipeline GetPipeline() const { return m_Pipeline; }
        VkPipelineLayout GetPipelineLayout() const { return m_PipelineLayout; }

        void Invalidate() override;
    private:
        VkPipeline m_Pipeline;
        VkPipelineLayout m_PipelineLayout;
    };
}
