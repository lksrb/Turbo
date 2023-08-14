#pragma once

#include "Turbo/Renderer/CommandQueue.h"

extern "C"
{
    // TODO: How does this work
#define VK_DEFINE_HANDLE(object) typedef struct object##_T* object;
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef struct object##_T *object;

VK_DEFINE_HANDLE(VkInstance)
VK_DEFINE_HANDLE(VkPhysicalDevice)
VK_DEFINE_HANDLE(VkDevice)
VK_DEFINE_HANDLE(VkQueue)
VK_DEFINE_HANDLE(VkCommandBuffer)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkCommandPool)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSurfaceKHR)
}

#define TBO_VK_ASSERT(x) { VkResult __result = (x); TBO_ENGINE_ASSERT(__result  == VK_SUCCESS, __result); }

namespace Turbo
{
    class Window;
    struct SwapchainSupportDetails;
    struct QueueFamilyIndices;
    
    class RendererContext
    {
    public:
        static void Init();
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
         * Resource queue manages vulkan handles on runtime, if vulkan wrappers are invalidated, vulkan handles will be submitted, to this queue
         * and safely released when the resource is not used.
         */
        template<typename F>
        static void SubmitResourceFree(F&& func)
        {
            GetResourceQueue().Submit(std::forward<F>(func));
        }

        // For now this is implemented just for FrameBuffer and Image2D since those are the only objects
        // that are getting resized
        template<typename F>
        static void SubmitRuntimeResourceFree(F&& func)
        {
            GetResourceRuntimeQueue().Submit(std::forward<F>(func));
        }
        static const SwapchainSupportDetails& GetSwapchainSupportDetails();
        static const QueueFamilyIndices& GetQueueFamilyIndices();
        static CommandQueue& GetResourceRuntimeQueue();
        static CommandQueue& GetResourceQueue();
    private:
        static void CreateInstance();
        static void CreateDebugger();
    };
}
