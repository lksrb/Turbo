#include "tbopch.h"
#include "GraphicsPipeline.h"

#include "Turbo/Platform/Vulkan/VulkanGraphicsPipeline.h"

namespace Turbo
{
    GraphicsPipeline::GraphicsPipeline(const GraphicsPipeline::Config& config)
        : m_Config(config)
    {
    }

    GraphicsPipeline::~GraphicsPipeline()
    {
    }

    Ref<GraphicsPipeline> GraphicsPipeline::Create(const GraphicsPipeline::Config& config)
    {
        return Ref<VulkanGraphicsPipeline>::Create(config);
    }
}
