#pragma once

#include "Turbo/Core/Common.h"
#include "Turbo/Renderer/ResourceQueue.h"

extern "C"
{
    // TODO: How does this work
#define VK_DEFINE_HANDLE(object) typedef struct object##_T* object;
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef struct object##_T *object;

VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkBuffer)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkImage)
VK_DEFINE_HANDLE(VkInstance)
VK_DEFINE_HANDLE(VkPhysicalDevice)
VK_DEFINE_HANDLE(VkDevice)
VK_DEFINE_HANDLE(VkQueue)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSemaphore)
VK_DEFINE_HANDLE(VkCommandBuffer)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkFence)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkDeviceMemory)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkEvent)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkQueryPool)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkBufferView)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkImageView)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkShaderModule)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkPipelineCache)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkPipelineLayout)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkPipeline)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkRenderPass)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkDescriptorSetLayout)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSampler)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkDescriptorSet)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkDescriptorPool)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkFramebuffer)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkCommandPool)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSurfaceKHR)
}

namespace Turbo
{
    class Window;
    struct SwapchainSupportDetails;
    struct QueueFamilyIndices;

    using SubmitCallback = void(*)(VkCommandBuffer);

    class RendererContext
    {
    public:
        static void Initialize(bool validationLayer = true, u32 framesInFlight = 3);
        static void Shutdown();

        static bool ValidationLayerEnabled();

        static VkInstance GetInstance();
        static VkDevice GetDevice();
        static VkPhysicalDevice GetPhysicalDevice();
        static VkCommandPool GetCommandPool();
        static VkSurfaceKHR GetSurface();
        static VkQueue GetPresentQueue();
        static VkQueue GetGraphicsQueue();

        static void WaitIdle();

        static VkCommandBuffer CreateCommandBuffer(bool begin = true);
        static void CreateSecondaryCommandBuffers(VkCommandBuffer* commandbuffers, u32 count);

        static void FlushCommandBuffer(VkCommandBuffer commandBuffer);
        static void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue);

        template<typename F>
        static void ImmediateSubmit(F&& func)
        {
            VkCommandBuffer commandBuffer = CreateCommandBuffer();
            func(commandBuffer);
            FlushCommandBuffer(commandBuffer);

            WaitIdle();
        }

        static void SetWindowContext(Window* window);

        static u32 FramesInFlight();

        /**
         * Resource queue manages vulkan handles on runtime, if vulkan wrappers are freed, vulkan handles will be submitted, to this queue, 
         * and safely released when the resource is not used.
         */
        static ResourceQueue& GetResourceQueue();
        static const SwapchainSupportDetails& GetSwapchainSupportDetails();
        static const QueueFamilyIndices& GetQueueFamilyIndices();
    private:
        static void CreateInstance();
        static void CreateDebugger();
    };


}
