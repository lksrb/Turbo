#include "tbopch.h"
#include "UniformBuffer.h"

#include "Renderer.h"
#include "RendererContext.h"

#include "Turbo/Platform/Vulkan/VulkanUniformBuffer.h"

namespace Turbo
{
    UniformBuffer::UniformBuffer(const UniformBuffer::Config& config)
        : m_Config(config)
    {
    }

    UniformBuffer::~UniformBuffer()
    {
        m_Data = nullptr;
    }

    Ref<UniformBuffer> UniformBuffer::Create(const UniformBuffer::Config& config)
    {
        return Ref<VulkanUniformBuffer>::Create(config);
    }

    UniformBufferSet::UniformBufferSet()
    {
    }

    UniformBufferSet::~UniformBufferSet()
    {
    }

    Ref<UniformBufferSet> UniformBufferSet::Create()
    {
        return Ref<UniformBufferSet>::Create();
    }

    void UniformBufferSet::SetData(u32 set, u32 binding, const void* data)
    {
        u32 current_frame = Renderer::GetCurrentFrame();
        // Uniform buffer at specific frame, at specific binding and at specific set (WOO!)
        m_UniformBufferMap.at(set).at(binding).at(current_frame)->SetData(data);
    }

    void UniformBufferSet::Create(u32 set, u32 binding, size_t data_size)
    {
        // Create or recreate uniform buffer
        u32 framesInFlight = RendererContext::FramesInFlight();

        auto& uniformBuffers = m_UniformBufferMap[set][binding];
        uniformBuffers.resize(framesInFlight);
        for (u32 i = 0; i < framesInFlight; ++i)
        {
            auto& uniformBuffer = m_UniformBufferMap[set][binding][i];
            uniformBuffer = UniformBuffer::Create({ set, binding, data_size });
            uniformBuffer->Invalidate();
        }
    }

}
