#pragma once

#include "Turbo/Core/PrimitiveTypes.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include <vector>
#include <optional>
#include <unordered_map>

namespace Turbo
{
    using ThreadID = unsigned int; // Corresponds to 

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> GraphicsFamily;
        std::optional<uint32_t> PresentFamily;

        bool IsComplete()
        {
            return GraphicsFamily.has_value() && PresentFamily.has_value();
        }
    };

    struct SwapchainSupportDetails
    {
        u32 nMinImageCount;
        VkSurfaceFormatKHR surfaceFormat;
        VkPresentModeKHR presentMode;
        VkSurfaceCapabilitiesKHR capabilities;
        VkPhysicalDeviceProperties properties;
    };

    class GPUDevice
    {
    public:
        GPUDevice();
        ~GPUDevice();

        void Initialize();
        void Shutdown();

        VkDevice GetDevice() const { return m_Device; }
        VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
        VkCommandPool GetCommandPool();

        VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
        VkQueue GetPresentQueue() const { return m_PresentQueue; }

        const SwapchainSupportDetails& GetSwapchainSupportDetails() const { return m_SupportDetails; }

        const QueueFamilyIndices& GetQueueFamilyIndices() const { return m_QueueFamilyIndices; }

        void WaitIdle();
    private:
        void CreatePhysicalDevice();
        void CreateLogicalDevice();

        void PopulateSwapchainSupportDetails();
        VkSurfaceFormatKHR ChooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        bool IsDeviceSuitable(VkPhysicalDevice device);
    private:
        VkDevice m_Device;
        VkPhysicalDevice m_PhysicalDevice;
        VkCommandPool m_CommandPool;

        VkQueue m_GraphicsQueue;
        VkQueue m_PresentQueue;

        QueueFamilyIndices m_QueueFamilyIndices;
        SwapchainSupportDetails m_SupportDetails;
    };
}
