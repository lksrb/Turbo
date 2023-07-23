#pragma once

#include "Turbo/Renderer/RenderPass.h"

#include <vulkan/vulkan.h>

namespace Turbo
{
    class VulkanRenderPass : public RenderPass
    {
    public:
        VulkanRenderPass(const RenderPass::Config& config);
        virtual ~VulkanRenderPass();

        VkRenderPass GetHandle() const { return m_RenderPass; }
        void Invalidate() override;
    private:
        VkRenderPass m_RenderPass = VK_NULL_HANDLE;
    };
}
