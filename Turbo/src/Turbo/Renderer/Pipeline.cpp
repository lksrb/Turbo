#include "tbopch.h"
#include "Pipeline.h"

#include "Turbo/Platform/Vulkan/VulkanPipeline.h"

namespace Turbo {

    Pipeline::Pipeline(const Pipeline::Config& config)
        : m_Config(config)
    {
    }

    Pipeline::~Pipeline()
    {
    }

    Ref<Pipeline> Pipeline::Create(const Pipeline::Config& config)
    {
        return Ref<VulkanPipeline>::Create(config);
    }
}
