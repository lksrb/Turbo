#pragma once

#include "Turbo/Renderer/RendererContext.h"
#include "Turbo/Renderer/RendererSettings.h"

#include "VulkanDevice.h"

#include <vulkan/vulkan.h>

#define TBO_VK_ASSERT(x) { VkResult __result = (x); TBO_ENGINE_ASSERT(__result  == VK_SUCCESS, __result); }

namespace Turbo {

    class VulkanContext : public RendererContext
    {
    public:
        void Initialize() override;
        void Shutdown() override;
        static VulkanContext* Get();

        VkInstance GetInstance() const { return m_Instance; }
        VkSurfaceKHR GetSurface() const { return m_Surface; }

        const VkSurfaceCapabilitiesKHR& GetSurfaceCapabilities() const { return m_SurfaceCapabilities; }
        const VkSurfaceFormatKHR& GetSurfaceFormat() const { return m_SurfaceFormat; }
        VkPresentModeKHR GetSurfacePresentMode() const { return m_SurfacePresentMode; }

        VulkanDevice& GetDevice() { return m_Device; }
    private:
        void CreateInstance();
        void CreateDebugMessenger();
        void CreateWindowSurface();
        void QuerySurfaceSupportDetails();
    private:
        VulkanDevice m_Device;

        VkSurfaceCapabilitiesKHR m_SurfaceCapabilities;
        VkSurfaceFormatKHR m_SurfaceFormat;
        VkPresentModeKHR m_SurfacePresentMode;

        VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
        VkInstance m_Instance = VK_NULL_HANDLE;
    };

}
