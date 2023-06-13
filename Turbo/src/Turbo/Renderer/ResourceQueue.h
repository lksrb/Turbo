#pragma once

#include "Turbo/Core/PrimitiveTypes.h"

#include <vector>
#include <functional>

namespace Turbo {

    // In ascending order
    enum ExecutionPriority : u32 
    {
        FRAMEBUFFER = 0,
        IMAGEVIEW,
        SWAPCHAIN,
        PIPELINE,
        PIPELINE_LAYOUT,
        RENDERPASS,
        BUFFER,
        DESCRIPTOR_POOL,
        SAMPLER,
        IMAGE,
        DESCRIPTOR_SET_LAYOUT,
        SHADER_MODULE,
        SYNC_OBJECT,
        COMMAND_POOL,
        DEVICE,
        DEBUG_MESSENGER,
        SURFACE,
        INSTANCE
    };

    enum class ExecutionOrder : u32 
    {
        Allocate = 0,
        Free,
    };

    struct ResourceCommand 
    {
        ExecutionPriority Priority;
        std::function<void()> Function;

        ResourceCommand(ExecutionPriority priority, std::function<void()>&& function)
            : Priority(priority), Function(std::move(function))
        {
        }

        bool operator<(const ResourceCommand& other) const
        {
            return (Priority < other.Priority);
        }
    };

    class ResourceQueue 
    {
    public:
        ResourceQueue();
        ~ResourceQueue();

        ResourceQueue(const ResourceQueue&) = delete;
        const ResourceQueue& operator=(const ResourceQueue& other) = delete;

        template<class F>
        void Submit(ExecutionPriority priority, F&& function)
        {
            m_Queue.emplace_back(priority, std::move(function));
        }

        bool Execute(ExecutionOrder order);

        size_t Size() const { return m_Queue.size(); }
    private:
        std::vector<ResourceCommand> m_Queue;
    };

}
