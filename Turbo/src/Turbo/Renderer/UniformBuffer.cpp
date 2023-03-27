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

    UniformBufferSet::UniformBufferSet(const UniformBufferSet::Config& config)
        : m_Config(config)
    {
        m_Buffers.resize(RendererContext::FramesInFlight());

        for (size_t i = 0; i < m_Buffers.size(); ++i)
        {
            m_Buffers[i] = UniformBuffer::Create({ m_Config.Binding, m_Config.Set, m_Config.Size });
            m_Buffers[i]->Invalidate();
        }
    }

    UniformBufferSet::~UniformBufferSet()
    {
    }

    void UniformBufferSet::SetData(const void* data)
    {
        u32 current_frame = Renderer::GetCurrentFrame();
        m_Buffers[current_frame]->SetData(data);
    }

    Ref<UniformBufferSet> UniformBufferSet::Create(const UniformBufferSet::Config& config)
    {
        return Ref<UniformBufferSet>::Create(config);
    }

}
