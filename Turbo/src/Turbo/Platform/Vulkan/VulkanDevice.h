#pragma once

#include <vulkan/vulkan.h>
#include <unordered_set>

namespace Turbo {

    class VulkanPhysicalDevice
    {
    public:
        struct QueueFamilyIndices
        {
            std::optional<u32> Graphics;
            std::optional<u32> Present;
            std::optional<u32> Compute;

            bool IsComplete()
            {
                return Graphics.has_value() && Present.has_value() && Compute.has_value();
            }
        };

        VulkanPhysicalDevice() = default;
        ~VulkanPhysicalDevice() = default;

        const QueueFamilyIndices& GetQueueFamilyIndices() const { return m_QueueFamilyIndices; }
        VkPhysicalDevice GetSelectedDevice() const { return m_PhysicalDevice; }
        const VkPhysicalDeviceProperties& GetProperties() const { return m_Properties; }
        const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const { return m_MemoryProperties; }
        operator VkPhysicalDevice() const { return m_PhysicalDevice; }

        void Select();
    private:
        bool IsDeviceSuitable(VkPhysicalDevice device);

        std::unordered_set<std::string> m_SupportedExtensions;
        QueueFamilyIndices m_QueueFamilyIndices;

        VkPhysicalDeviceMemoryProperties m_MemoryProperties;
        VkPhysicalDeviceProperties m_Properties;
        VkPhysicalDeviceFeatures m_Features;
        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    };

    class VulkanDevice
    {
    public:
        VulkanDevice() = default;
        ~VulkanDevice() = default;

        VulkanDevice(const VulkanDevice&) = delete;

        void Init();
        void Shutdown();

        void WaitIdle() const;

        VkCommandBuffer CreateCommandBuffer(bool begin = true);
        void FlushCommandBuffer(VkCommandBuffer commandBuffer);
        void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue);

        VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
        VkQueue GetPresentQueue() const { return m_PresentQueue; }
        VkQueue GetComputeQueue() const { return m_ComputeQueue; }
        VkCommandPool GetCommandPool() const { return m_CommandPool; }
        VkDevice GetHandle() const { return m_Handle; }

        VulkanPhysicalDevice& GetPhysicalDevice() { return m_PhysicalDevice; }

        operator VkDevice() const { return m_Handle;  }
    private:
        VulkanPhysicalDevice m_PhysicalDevice;

        VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
        VkQueue m_PresentQueue = VK_NULL_HANDLE;
        VkQueue m_ComputeQueue = VK_NULL_HANDLE;
        VkCommandPool m_CommandPool = VK_NULL_HANDLE;
        VkDevice m_Handle = VK_NULL_HANDLE;
    };

}
