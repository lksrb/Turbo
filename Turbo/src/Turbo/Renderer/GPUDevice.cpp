#include "tbopch.h"

#include "GPUDevice.h"

#include "Turbo/Renderer/RendererContext.h"

namespace Turbo {

    void GPUDevice::Initialize()
    {
        CreatePhysicalDevice();
        CreateLogicalDevice();
    }

    void GPUDevice::Shutdown()
    {
        vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
        vkDestroyDevice(m_Device, nullptr);
    }

	VkCommandPool GPUDevice::GetCommandPool()
	{
        return m_CommandPool;
	}

	void GPUDevice::WaitIdle()
	{
        TBO_VK_ASSERT(vkDeviceWaitIdle(m_Device));
	}

	// Physical device

    void GPUDevice::CreatePhysicalDevice()
    {
        VkInstance instance = RendererContext::GetInstance();

        uint32_t deviceCount = 0;
        TBO_VK_ASSERT(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));
        TBO_ENGINE_ASSERT(deviceCount, "Failed to find GPUs with Vulkan support!");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        TBO_VK_ASSERT(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()));

        for (const auto& device : devices)
        {
            if (IsDeviceSuitable(device))
            {
                m_PhysicalDevice = device;
                break;
            }
        }
        TBO_ENGINE_ASSERT(m_PhysicalDevice, "Failed to find suitable GPU!");

        PopulateSwapchainSupportDetails();
    }


    bool GPUDevice::IsDeviceSuitable(VkPhysicalDevice device)
    {
        VkSurfaceKHR surface = RendererContext::GetSurface();

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        i32 i = 0;
        for (const auto& queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                m_QueueFamilyIndices.GraphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            TBO_VK_ASSERT(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport));

            if (presentSupport)
                m_QueueFamilyIndices.PresentFamily = i;

            if (m_QueueFamilyIndices.IsComplete())
                break;

            i++;
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return m_QueueFamilyIndices.IsComplete() && supportedFeatures.samplerAnisotropy;
    }

    void GPUDevice::PopulateSwapchainSupportDetails()
    {
        VkSurfaceKHR surface = RendererContext::GetSurface();

        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;

        // Query for swap chain capabilities
        TBO_VK_ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, surface, &capabilities));

        // Query for swap chain available formats
        uint32_t formatCount;
        TBO_VK_ASSERT(vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, surface, &formatCount, nullptr));

        if (formatCount != 0)
        {
            formats.resize(formatCount);
            TBO_VK_ASSERT(vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, surface, &formatCount, formats.data()));
        }
        // Query for swap chain available present modes
        uint32_t presentModeCount;
        TBO_VK_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, surface, &presentModeCount, nullptr));

        if (presentModeCount != 0)
        {
            presentModes.resize(presentModeCount);
            TBO_VK_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, surface, &presentModeCount, presentModes.data()));
        }

        VkPhysicalDeviceProperties properties = {};
        vkGetPhysicalDeviceProperties(m_PhysicalDevice, &properties);
        m_SupportDetails.Properties = properties;

        m_SupportDetails.SurfaceFormat = ChooseSwapchainSurfaceFormat(formats);
        m_SupportDetails.PresentMode = ChooseSwapchainPresentMode(presentModes);
        m_SupportDetails.Capabilities = capabilities;
        
        TBO_ENGINE_ASSERT(properties.limits.maxPushConstantsSize >= 64, "Turbo Engine require 64 bytes minimum for push constants!");

        TBO_ENGINE_ASSERT(capabilities.minImageCount > 0);
        m_SupportDetails.MinImageCount = std::min(capabilities.minImageCount, capabilities.maxImageCount);
    }

    VkSurfaceFormatKHR GPUDevice::ChooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        for (const auto& availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }
    VkPresentModeKHR GPUDevice::ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        for (const auto& availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    // Logical device

    void GPUDevice::CreateLogicalDevice()
    {
        const auto& indices = m_QueueFamilyIndices;

        // Manage queues
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<u32> uniqueQueueFamilies = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };

        float queuePriority = 1.0f;
        for (auto& queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        std::vector<const char*> extensions {
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
        VkPhysicalDeviceFeatures2 deviceFeatures2{};
        deviceFeatures2.features = deviceFeatures;
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures2.pNext = &robustnessFeature2;

        /*	VkPhysicalDeviceFeatures2 deviceFeatures2{};
            deviceFeatures2.features = robustnessFeature;*/
            // Manage device
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = nullptr;
        createInfo.pNext = &deviceFeatures2;
        createInfo.enabledLayerCount = 0;

        createInfo.enabledExtensionCount = static_cast<u32>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        // Validation layers used for debugging Vulkan
        std::vector<const char*> validationLayers;

        if (RendererContext::ValidationLayerEnabled())
        {
            validationLayers.push_back("VK_LAYER_KHRONOS_validation");

            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }

        TBO_VK_ASSERT(vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device));

        vkGetDeviceQueue(m_Device, indices.GraphicsFamily.value(), 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_Device, indices.PresentFamily.value(), 0, &m_PresentQueue);

        // Command pool
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = indices.GraphicsFamily.value();

        TBO_VK_ASSERT(vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool));
    }

}
