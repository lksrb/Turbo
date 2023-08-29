#include "tbopch.h"
#include "VulkanDevice.h"

#include "VulkanContext.h"

#include "Turbo/Renderer/RendererContext.h"

namespace Turbo {

    // Physical device
    // Physical device
    // Physical device

    void VulkanPhysicalDevice::Select()
    {
        VkInstance instance = VulkanContext::Get()->GetInstance();

        u32 deviceCount = 0;
        TBO_VK_ASSERT(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));
        TBO_ENGINE_ASSERT(deviceCount, "Failed to find GPUs with Vulkan support!");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        TBO_VK_ASSERT(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()));

        // Select device
        for (auto device : devices)
        {
            if (IsDeviceSuitable(device))
            {
                m_PhysicalDevice = device;
                break;
            }
        }

        TBO_ENGINE_ASSERT(m_PhysicalDevice, "Failed to find suitable GPU!");

        // Query device features
        vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &m_Features);
        vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_Properties);
        vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &m_MemoryProperties);
        
        // Query available extensions
        u32 extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(m_PhysicalDevice,nullptr, &extensionCount, nullptr);
        if (extensionCount > 0)
        {
            std::vector<VkExtensionProperties> extensions(extensionCount);
            if (vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionCount, extensions.data()) == VK_SUCCESS)
            {
                TBO_ENGINE_TRACE("Selected GPU has {} extensions.", extensions.size());
                m_SupportedExtensions.reserve(extensions.size());
                for (const auto& ext : extensions)
                {
                    m_SupportedExtensions.emplace(ext.extensionName);
                    TBO_ENGINE_TRACE("  {}", ext.extensionName);
                }
            }
        }
    }

    bool VulkanPhysicalDevice::IsDeviceSuitable(VkPhysicalDevice device)
    {
        VkSurfaceKHR surface = VulkanContext::Get()->GetSurface();

        u32 queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        
        i32 i = 0;
        for (const auto& queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                m_QueueFamilyIndices.Graphics = i;
            }

            if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
            {
                m_QueueFamilyIndices.Compute = i;
            }

            VkBool32 presentSupport = false;
            TBO_VK_ASSERT(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport));
            if (presentSupport)
                m_QueueFamilyIndices.Present = i;

            if (m_QueueFamilyIndices.IsComplete())
                break;

            i++;
        }

        return m_QueueFamilyIndices.IsComplete();
    }

    // Logical device
    // Logical device
    // Logical device

    void VulkanDevice::Init()
    {
        m_PhysicalDevice.Select();
        
        const auto& indices = m_PhysicalDevice.GetQueueFamilyIndices();

        // Manage queues
        std::set<u32> uniqueQueueFamilies = { indices.Graphics.value(), indices.Present.value(), indices.Compute.value() };
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        f32 queuePriority = 1.0f;
        for (auto& queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        std::vector<const char*> extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_EXT_ROBUSTNESS_2_EXTENSION_NAME,
            VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME
        };
        VkPhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.robustBufferAccess = VK_TRUE;
        deviceFeatures.wideLines = VK_TRUE;
        deviceFeatures.fillModeNonSolid = VK_TRUE;
        deviceFeatures.independentBlend = VK_TRUE;

        VkPhysicalDeviceRobustness2FeaturesEXT robustnessFeature2 = {};
        robustnessFeature2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT;
        robustnessFeature2.robustBufferAccess2 = VK_TRUE;
        robustnessFeature2.robustImageAccess2 = VK_FALSE;
        robustnessFeature2.nullDescriptor = VK_TRUE;
        VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
        deviceFeatures2.features = deviceFeatures;
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures2.pNext = &robustnessFeature2;

        // Create device
        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = nullptr;
        createInfo.pNext = &deviceFeatures2;
        createInfo.enabledLayerCount = 0;

        createInfo.enabledExtensionCount = static_cast<u32>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        // Validation layers used for debugging Vulkan
        std::array<const char*, 1> validationLayers;

        if constexpr (RendererSettings::EnableValidationLayers)
        {
            validationLayers[0] = "VK_LAYER_KHRONOS_validation";

            createInfo.enabledLayerCount = static_cast<u32>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }

        TBO_VK_ASSERT(vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Handle));

        vkGetDeviceQueue(m_Handle, indices.Graphics.value(), 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_Handle, indices.Present.value(), 0, &m_PresentQueue);
        vkGetDeviceQueue(m_Handle, indices.Compute.value(), 0, &m_ComputeQueue);

        // Create Command pool
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = indices.Graphics.value();

        TBO_VK_ASSERT(vkCreateCommandPool(m_Handle, &poolInfo, nullptr, &m_CommandPool));
    }

    void VulkanDevice::Shutdown()
    {
        vkDestroyCommandPool(m_Handle, m_CommandPool, nullptr);
        vkDestroyDevice(m_Handle, nullptr);
    }

    void VulkanDevice::WaitIdle() const
    {
        vkDeviceWaitIdle(m_Handle);
    }

    VkCommandBuffer VulkanDevice::CreateCommandBuffer(bool begin)
    {
        VkCommandBuffer cmdBuffer;

        VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
        cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdBufAllocateInfo.commandPool = m_CommandPool;
        cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdBufAllocateInfo.commandBufferCount = 1;

        TBO_VK_ASSERT(vkAllocateCommandBuffers(m_Handle, &cmdBufAllocateInfo, &cmdBuffer));

        // If requested, also start the new command buffer
        if (begin)
        {
            VkCommandBufferBeginInfo cmdBufferBeginInfo{};
            cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            TBO_VK_ASSERT(vkBeginCommandBuffer(cmdBuffer, &cmdBufferBeginInfo));
        }

        return cmdBuffer;
    }

    void VulkanDevice::FlushCommandBuffer(VkCommandBuffer commandBuffer)
    {
        FlushCommandBuffer(commandBuffer, m_GraphicsQueue);
    }

    void VulkanDevice::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue)
    {
        TBO_ENGINE_ASSERT(commandBuffer != VK_NULL_HANDLE);

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
        TBO_VK_ASSERT(vkCreateFence(m_Handle, &fenceCreateInfo, nullptr, &fence));

        // Submit to the queue
        TBO_VK_ASSERT(vkQueueSubmit(queue, 1, &submitInfo, fence));

        // Wait for the fence to signal that command buffer has finished executing
        TBO_VK_ASSERT(vkWaitForFences(m_Handle, 1, &fence, VK_TRUE, UINT64_MAX));

        vkDestroyFence(m_Handle, fence, nullptr);
        vkFreeCommandBuffers(m_Handle, m_CommandPool, 1, &commandBuffer);
    }
}
