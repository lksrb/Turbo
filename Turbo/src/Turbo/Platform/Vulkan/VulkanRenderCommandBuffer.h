#pragma once

#include "Turbo/Renderer/RenderCommandBuffer.h"
#include "Turbo/Renderer/Fly.h"

#include <vulkan/vulkan.h>

namespace Turbo
{
    class VulkanRenderCommandBuffer : public RenderCommandBuffer
    {
    public:
        VulkanRenderCommandBuffer();
        ~VulkanRenderCommandBuffer();

        VkCommandBuffer GetHandle() const;

        void Begin() override;
        void End() override;

        void Submit() override;
    private:
        Fly<VkFence> m_WaitFences;
        Fly<VkCommandBuffer> m_CommandBuffers;
    };
}
