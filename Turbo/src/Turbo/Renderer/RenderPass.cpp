#include "tbopch.h"
#include "RenderPass.h"

#include "Turbo/Platform/Vulkan/VulkanRenderPass.h"

namespace Turbo
{
    RenderPass::RenderPass(const RenderPass::Config& config)
        : m_Config(config)
    {
    }

    Ref<RenderPass> RenderPass::Create(const RenderPass::Config& config)
    {
        return Ref<VulkanRenderPass>::Create(config);
    }

    RenderPass::~RenderPass()
    {
    }
}
