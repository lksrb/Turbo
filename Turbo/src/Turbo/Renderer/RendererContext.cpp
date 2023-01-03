#include "tbopch.h"
#include "RendererContext.h"

#include "GPUDevice.h"

#include "Turbo/Core/Engine.h"

#ifdef TBO_PLATFORM_WIN32
    #include "Turbo/Platform/Win32/Win32_Window.h"
    #define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.h>
#include <vector>

namespace Turbo
{
    namespace Utils
    {
        static bool CheckValidationLayerSupport(const std::vector<const char*>& layers)
        {
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

            for (auto layerName : layers)
            {
                bool layerFound = false;

                for (const auto& layerProperties : availableLayers)
                {
                    if (strcmp(layerName, layerProperties.layerName) == 0)
                    {
                        layerFound = true;
                        break;
                    }
                }

                if (!layerFound)
                {
                    return false;
                }
            }
            return true;
        }
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {

        switch (messageSeverity)
        {
            //case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: TBO_ENGINE_TRACE("Validation layer: {0}", pCallbackData->pMessage); break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:    TBO_ENGINE_INFO("Validation layer: {0}", pCallbackData->pMessage); break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: TBO_ENGINE_WARN("Validation layer: {0}", pCallbackData->pMessage); break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:   TBO_ENGINE_ERROR("Validation layer: {0}", pCallbackData->pMessage); break;
            //	default:
                    //PG_CORE_ASSERT(false); // Invalid severity
        }

        return VK_FALSE;
    }

    struct RendererContextInternal
    {
        VkInstance                  Instance;
        VkSurfaceKHR                Surface;
        VkDebugUtilsMessengerEXT    DebugMessenger; // TODO: Replace vulkan function calls with function pointers.(Vulkan loader)
        GPUDevice                   Device;

        ResourceQueue               RuntimeResourceQueue;          
        bool                        ValidationLayerEnabled;
        u32                         FramesInFlight;
    };

    static RendererContextInternal* I;

    void RendererContext::Initialize(bool validationLayer, u32 framesInFlight)
    {
        I = new RendererContextInternal{ };

        I->ValidationLayerEnabled = validationLayer;
        I->FramesInFlight = framesInFlight;

        CreateInstance();
        CreateDebugger();

        TBO_ENGINE_INFO("RendererContext initialized!");
    }

    void RendererContext::Shutdown()
    {
        GetResourceQueue().Execute(ExecutionOrder::Free);

        vkDestroySurfaceKHR(I->Instance, I->Surface, nullptr);

        I->Device.Shutdown();

        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(I->Instance, "vkDestroyDebugUtilsMessengerEXT");
        TBO_ENGINE_ASSERT(func);
        func(I->Instance, I->DebugMessenger, nullptr);

        vkDestroyInstance(I->Instance, nullptr);

        delete I;
    }

    bool RendererContext::ValidationLayerEnabled()
    {
        return I->ValidationLayerEnabled;
    }

    VkInstance RendererContext::GetInstance()
    {
        return I->Instance;
    }

    VkDevice RendererContext::GetDevice()
    {
        return I->Device.GetDevice();
    }

    VkPhysicalDevice RendererContext::GetPhysicalDevice()
    {
        return I->Device.GetPhysicalDevice();
    }

    VkCommandPool RendererContext::GetCommandPool()
    {
        return I->Device.GetCommandPool();
    }

    VkSurfaceKHR RendererContext::GetSurface()
    {
        TBO_ENGINE_ASSERT(I->Surface, "VkSurfaceKHR is nullptr! Call SetWindowContext first")
        return I->Surface;
    }

    VkQueue RendererContext::GetPresentQueue()
    {
        return I->Device.GetPresentQueue();
    }

    VkQueue RendererContext::GetGraphicsQueue()
    {
        return I->Device.GetGraphicsQueue();
    }

    void RendererContext::WaitIdle()
    {
        I->Device.WaitIdle();
    }

    VkCommandBuffer RendererContext::CreateCommandBuffer(bool begin)
    {
        VkCommandBuffer cmdBuffer;

        VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
        cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdBufAllocateInfo.commandPool = I->Device.GetCommandPool();
        cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdBufAllocateInfo.commandBufferCount = 1;

        TBO_VK_ASSERT(vkAllocateCommandBuffers(I->Device.GetDevice(), &cmdBufAllocateInfo, &cmdBuffer));

        // If requested, also start the new command buffer
        if (begin)
        {
            VkCommandBufferBeginInfo cmdBufferBeginInfo{};
            cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            TBO_VK_ASSERT(vkBeginCommandBuffer(cmdBuffer, &cmdBufferBeginInfo));
        }

        return cmdBuffer;
    }

    void RendererContext::CreateSecondaryCommandBuffers(VkCommandBuffer* commandbuffers, u32 count)
    {
        VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
        cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdBufAllocateInfo.commandPool = I->Device.GetCommandPool();
        cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        cmdBufAllocateInfo.commandBufferCount = count;

        TBO_VK_ASSERT(vkAllocateCommandBuffers(I->Device.GetDevice(), &cmdBufAllocateInfo, commandbuffers));
    }

    void RendererContext::FlushCommandBuffer(VkCommandBuffer commandBuffer)
    {
        FlushCommandBuffer(commandBuffer, I->Device.GetGraphicsQueue());
    }

    void RendererContext::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue)
    {
        TBO_ENGINE_ASSERT(commandBuffer != VK_NULL_HANDLE);

        VkDevice device = I->Device.GetDevice();

        TBO_VK_ASSERT(vkEndCommandBuffer(commandBuffer));

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        // Create fence to ensure that the command buffer has finished executing
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = 0;
        VkFence fence;
        TBO_VK_ASSERT(vkCreateFence(device, &fenceCreateInfo, nullptr, &fence));

        // Submit to the queue
        TBO_VK_ASSERT(vkQueueSubmit(queue, 1, &submitInfo, fence));

        // Wait for the fence to signal that command buffer has finished executing
        TBO_VK_ASSERT(vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX));

        vkDestroyFence(device, fence, nullptr);
        vkFreeCommandBuffers(device, I->Device.GetCommandPool(), 1, &commandBuffer);
    }

    void RendererContext::SetWindowContext(Window* window)
    {
#ifdef TBO_PLATFORM_WIN32
        Win32_Window* win32 = dynamic_cast<Win32_Window*>(Engine::Get().GetViewportWindow());
        
        VkWin32SurfaceCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hinstance = win32->GetInstance();
        createInfo.hwnd = win32->GetHandle();
        TBO_VK_ASSERT(vkCreateWin32SurfaceKHR(RendererContext::GetInstance(), &createInfo, nullptr, &I->Surface));
#endif
        I->Device.Initialize();

        window->InitializeSwapchain();
    }

    u32 RendererContext::FramesInFlight()
    {
        return I->FramesInFlight;
    }

    ResourceQueue& RendererContext::GetResourceQueue()
    {
        return I->RuntimeResourceQueue;
    }

    const SwapchainSupportDetails& RendererContext::GetSwapchainSupportDetails()
    {
        return (const SwapchainSupportDetails&)I->Device.GetSwapchainSupportDetails();
    }

    const QueueFamilyIndices& RendererContext::GetQueueFamilyIndices()
    {
        return (const QueueFamilyIndices&)I->Device.GetQueueFamilyIndices();
    }

    void RendererContext::CreateInstance()
    {
        // App info
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Turbo Editor";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Turbo Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        // Vulkan Instance
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

        std::vector<const char*> extensions;
        extensions.push_back("VK_KHR_surface");
#ifdef TBO_PLATFORM_WIN32
        extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
        std::vector<const char*> validationLayers;

        if (I->ValidationLayerEnabled)
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            extensions.push_back("VK_EXT_debug_report");

            // Validation layers
            validationLayers = {
                "VK_LAYER_KHRONOS_validation",
            };
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            {
                // For vkCreateInstance and vkDestroyInstance debug
                debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                debugCreateInfo.pfnUserCallback = DebugCallback;
            }

            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }

        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        TBO_VK_ASSERT(vkCreateInstance(&createInfo, nullptr, &I->Instance));
    }

    void RendererContext::CreateDebugger()
    {
        if (I->ValidationLayerEnabled == false)
            return;
        std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };

        TBO_ENGINE_ASSERT(Utils::CheckValidationLayerSupport(validationLayers));

        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = DebugCallback;
        createInfo.pUserData = nullptr; // Optional

        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(I->Instance, "vkCreateDebugUtilsMessengerEXT");
        TBO_ENGINE_ASSERT(func);
        TBO_VK_ASSERT(func(I->Instance, &createInfo, nullptr, &I->DebugMessenger));
    }
}
