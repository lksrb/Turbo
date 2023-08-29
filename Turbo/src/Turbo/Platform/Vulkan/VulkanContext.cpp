#include "tbopch.h"

#ifdef TBO_PLATFORM_WIN32
    #define VK_USE_PLATFORM_WIN32_KHR
#endif
#include "VulkanContext.h"

#include "Turbo/Core/Application.h"

#include "Turbo/Platform/Win32/Win32_Window.h"
#include "Turbo/Renderer/Renderer.h"

namespace Turbo {

    namespace Detail {

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

        // From imgui.h
        VkSurfaceFormatKHR SelectSurfaceFormat()
        {
            static VkColorSpaceKHR request_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

            static std::array<VkFormat, 4> request_formats = {
                VK_FORMAT_B8G8R8A8_UNORM,
                VK_FORMAT_B8G8R8A8_SRGB,
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_FORMAT_R8G8B8A8_UNORM,
            };

            VkPhysicalDevice physicalDevice = VulkanContext::Get()->GetDevice().GetPhysicalDevice();
            VkSurfaceKHR surface = VulkanContext::Get()->GetSurface();

            // Per Spec Format and View Format are expected to be the same unless VK_IMAGE_CREATE_MUTABLE_BIT was set at image creation
            // Assuming that the default behavior is without setting this bit, there is no need for separate Swapchain image and image view format
            // Additionally several new color spaces were introduced with Vulkan Spec v1.0.40,
            // hence we must make sure that a format with the mostly available color space, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, is found and used.
            u32 availCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &availCount, nullptr);
            std::vector<VkSurfaceFormatKHR> availFormats;
            availFormats.resize((u64)availCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &availCount, availFormats.data());

            // First check if only one format, VK_FORMAT_UNDEFINED, is available, which would imply that any format is available
            if (availCount == 1)
            {
                if (availFormats[0].format == VK_FORMAT_UNDEFINED)
                {
                    VkSurfaceFormatKHR ret;
                    ret.format = request_formats[0];
                    ret.colorSpace = request_color_space;
                    return ret;
                }
                else
                {
                    // No point in searching another format
                    return availFormats[0];
                }
            }
            else
            {
                // Request several formats, the first found will be used
                for (u64 request_i = 0; request_i < request_formats.size(); request_i++)
                    for (u32 avail_i = 0; avail_i < availCount; avail_i++)
                        if (availFormats[avail_i].format == request_formats[request_i] && availFormats[avail_i].colorSpace == request_color_space)
                            return availFormats[avail_i];

                // If none of the requested image formats could be found, use the first available
                return availFormats[0];
            }
        }

        VkPresentModeKHR ChooseSwapchainPresentMode()
        {
            VkPhysicalDevice physicalDevice = VulkanContext::Get()->GetDevice().GetPhysicalDevice().GetSelectedDevice();
            VkSurfaceKHR surface = VulkanContext::Get()->GetSurface();

            // Query for swap chain available present modes
            uint32_t presentModeCount;
            TBO_VK_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr));
            std::vector<VkPresentModeKHR> availablePresentModes;

            if (presentModeCount != 0)
            {
                availablePresentModes.resize(presentModeCount);
                TBO_VK_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, availablePresentModes.data()));
            }

            for (const auto& availablePresentMode : availablePresentModes)
            {
                if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                {
                    return availablePresentMode;
                }
            }

            return VK_PRESENT_MODE_FIFO_KHR;
        }

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
        {
            switch (messageSeverity)
            {
                //case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: TBO_ENGINE_TRACE("Validation layer: {0}", pCallbackData->pMessage); break;
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:    TBO_ENGINE_INFO("Validation layer: {0}", pCallbackData->pMessage); break;
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: TBO_ENGINE_WARN("Validation layer: {0}", pCallbackData->pMessage); break;
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:   TBO_ENGINE_ERROR("Validation layer: {0}", pCallbackData->pMessage); break;
            }

            return VK_FALSE;
        }

    }

    void VulkanContext::Initialize()
    {
        // Create instance context
        CreateInstance();

        // Create a debug logger and enable validation layers
        CreateDebugMessenger();

        // Connect platform-specific window surface to Vulkan
        CreateWindowSurface();

        // Create physical and logical device
        m_Device.Init();

        // Query surface details such as available present modes, surface formats, ...
        QuerySurfaceSupportDetails();

        TBO_ENGINE_INFO("VulkanContext initialized!");
    }

    void VulkanContext::Shutdown()
    {
        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);

        m_Device.Shutdown();

        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT");
        TBO_ENGINE_ASSERT(func);
        func(m_Instance, m_DebugMessenger, nullptr);

        vkDestroyInstance(m_Instance, nullptr);
    }

    Ref<VulkanContext> VulkanContext::Get()
    {
        return Renderer::GetContext();
    }

    void VulkanContext::CreateInstance()
    {
        // App info
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Turbo Engine";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Turbo Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        // Vulkan Instance
        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};

        std::vector<const char*> extensions;
        extensions.push_back("VK_KHR_surface");
#ifdef TBO_PLATFORM_WIN32
        extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
        std::vector<const char*> validationLayers;

        if constexpr (RendererSettings::EnableValidationLayers)
        {
            extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

            // Validation layers
            validationLayers.push_back("VK_LAYER_KHRONOS_validation");

            createInfo.enabledLayerCount = static_cast<u32>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            {
                // For vkCreateInstance and vkDestroyInstance debug
                debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                debugCreateInfo.pfnUserCallback = Detail::DebugCallback;
            }

            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }

        createInfo.enabledExtensionCount = static_cast<u32>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        TBO_VK_ASSERT(vkCreateInstance(&createInfo, nullptr, &m_Instance));
    }

    void VulkanContext::CreateDebugMessenger()
    {
        std::vector<const char*> validationLayers;
        validationLayers.push_back("VK_LAYER_KHRONOS_validation");

        TBO_ENGINE_ASSERT(Detail::CheckValidationLayerSupport(validationLayers));

        VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = Detail::DebugCallback;
        createInfo.pUserData = nullptr; // Optional

        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT");
        TBO_ENGINE_ASSERT(func);
        TBO_VK_ASSERT(func(m_Instance, &createInfo, nullptr, &m_DebugMessenger));
    }

    void VulkanContext::CreateWindowSurface()
    {
#ifdef TBO_PLATFORM_WIN32
        Win32_Window* win32 = dynamic_cast<Win32_Window*>(Application::Get().GetViewportWindow());

        VkWin32SurfaceCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hinstance = win32->GetInstance();
        createInfo.hwnd = win32->GetHandle();
        TBO_VK_ASSERT(vkCreateWin32SurfaceKHR(m_Instance, &createInfo, nullptr, &m_Surface));
#endif
    }

    void VulkanContext::QuerySurfaceSupportDetails()
    {
        // Query for swap chain capabilities
        TBO_VK_ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_Device.GetPhysicalDevice(), m_Surface, &m_SurfaceCapabilities));

        m_SurfaceFormat = Detail::SelectSurfaceFormat();
        m_SurfacePresentMode = Detail::ChooseSwapchainPresentMode();
    }

}
