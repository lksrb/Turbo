#include "tbopch.h"
#include "VulkanUserInterface.h"

#include "Turbo/Core/Engine.h"
#include "Turbo/Core/Platform.h"
#include "Turbo/UI/ImGuiStyler.h"

#include "Turbo/Renderer/GPUDevice.h"

#include "IconsFontAwesome6.h"

#ifdef TBO_PLATFORM_WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#include "Turbo/Platform/Win32/Win32_Window.h"
#endif

#include "Turbo/Platform/Vulkan/VulkanSwapChain.h"

#pragma warning(push, 0)
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_vulkan.h>
#include <ImGuizmo.h>
#pragma warning(pop)

// Copy this line into your .cpp file to forward declare the function.
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Turbo
{
#ifdef TBO_PLATFORM_WIN32
    static int ImGui_ImplWin32_CreateVkSurface(ImGuiViewport* viewport, ImU64 vkInstance, const void* vkAllocator, ImU64* outVkSurface)
    {
        ImGuiIO& io = ImGui::GetIO();
        VkWin32SurfaceCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hinstance = ::GetModuleHandle(NULL);
        createInfo.hwnd = (HWND)viewport->PlatformHandleRaw;

        VkResult err = vkCreateWin32SurfaceKHR((VkInstance)vkInstance, &createInfo, (const VkAllocationCallbacks*)vkAllocator, (VkSurfaceKHR*)outVkSurface);
        return err;
        return -1;
    }
#endif

    namespace Utils
    {
        static ImGuiMouseCursor ImGuiCursorToCursorMode(CursorMode cursorMode)
        {
            switch (cursorMode)
            {
                case Turbo::CursorMode_Hidden: return ImGuiMouseCursor_None;
                case Turbo::CursorMode_Arrow: return ImGuiMouseCursor_Arrow;
                case Turbo::CursorMode_Hand: return ImGuiMouseCursor_Hand;
            }
            
            TBO_ENGINE_ERROR("Invalid cursor mode!");

            return ImGuiMouseCursor_None;
        }

    }

    static void CheckVkResult(VkResult r)
    {
        TBO_VK_ASSERT(r);
    }

    VulkanUserInterface::VulkanUserInterface()
        : UserInterface()
    {
        CreateImGuiContext();
    }

    VulkanUserInterface::~VulkanUserInterface()
    {
        VkDevice device = RendererContext::GetDevice();

        vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplWin32_Shutdown();

        ImGui::DestroyContext();
    }

    void VulkanUserInterface::CreateImGuiContext()
    {
        VkDevice device = RendererContext::GetDevice();
        u32 framesInFlight = RendererContext::FramesInFlight();
        Window* viewportWindow = Engine::Get().GetViewportWindow();

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;    // Enable mouse input
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;			// Enable Multi-Viewport / Platform Windows
        //io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;     // Disable this so Input class can control mouse cursor
        io.Fonts->AddFontDefault();

        // Merge in icons from Font Awesome
        static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
        ImFontConfig icons_config;
        icons_config.MergeMode = true;
        icons_config.PixelSnapH = true;
        icons_config.GlyphOffset.y = 3;
        io.Fonts->AddFontFromFileTTF("Resources/Fonts/IconFonts/" FONT_ICON_FILE_NAME_FAR, 16.0f, &icons_config, icons_ranges);
        //io.FontGlobalScale = 1.5f;

        // Styles are in separated class so the recompilation is as fast as possible
        // TODO: Probably some color tweak window with everything so I can really style it
        ImGuiStyler::Style();

        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
        // Allocate descriptor pool
        VkDescriptorPoolSize poolSizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100 }
        };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 100 * IM_ARRAYSIZE(poolSizes);
        pool_info.poolSizeCount = (u32)IM_ARRAYSIZE(poolSizes);
        pool_info.pPoolSizes = poolSizes;
        TBO_VK_ASSERT(vkCreateDescriptorPool(device, &pool_info, nullptr, &m_DescriptorPool));
#ifdef TBO_PLATFORM_WIN32
        Win32_Window* window = dynamic_cast<Win32_Window*>(viewportWindow);
        TBO_ENGINE_ASSERT(ImGui_ImplWin32_Init(window->GetHandle()));
#endif
        // Custom CreateVkSurface function
        ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
        platform_io.Platform_CreateVkSurface = ImGui_ImplWin32_CreateVkSurface;

        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.Instance = RendererContext::GetInstance();
        initInfo.PhysicalDevice = RendererContext::GetPhysicalDevice();
        initInfo.Device = RendererContext::GetDevice();
        initInfo.QueueFamily = RendererContext::GetQueueFamilyIndices().GraphicsFamily.value();
        initInfo.Queue = RendererContext::GetGraphicsQueue();
        initInfo.PipelineCache = nullptr;
        initInfo.DescriptorPool = m_DescriptorPool;
        initInfo.Subpass = 0;
        initInfo.MinImageCount = framesInFlight; // Swapchain image count
        initInfo.ImageCount = framesInFlight; // Swapchain image count
        initInfo.Allocator = nullptr;
        initInfo.CheckVkResultFn = CheckVkResult;

        Ref<VulkanSwapChain> swapchain = viewportWindow->GetSwapchain().As<VulkanSwapChain>();
        VkRenderPass renderPass = swapchain->GetRenderPass();

        ImGui_ImplVulkan_Init(&initInfo, renderPass);

        // Submits and wait till the command buffer is finished
        RendererContext::ImmediateSubmit([](VkCommandBuffer commandBuffer)
        {
            ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
        });

        ImGui_ImplVulkan_DestroyFontUploadObjects();

        // Create secondary command buffers for each frame in flight
        RendererContext::CreateSecondaryCommandBuffers(m_SecondaryBuffers.Data(), framesInFlight);
    }
    void VulkanUserInterface::BeginUI()
    {
        ImGui_ImplWin32_NewFrame();
        ImGui_ImplVulkan_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();

        // FIXME: Maybe not ideal solution
        ImGui::SetMouseCursor(Utils::ImGuiCursorToCursorMode(Input::GetCursorMode()));
    }

    void VulkanUserInterface::EndUI()
    {
        ImGui::Render();

        // Swapchain primary command buffer
        {
            const Window* viewportWindow = Engine::Get().GetViewportWindow();
            Ref<VulkanSwapChain> swapChain = viewportWindow->GetSwapchain().As<VulkanSwapChain>();
            u32 width = viewportWindow->GetWidth();
            u32 height = viewportWindow->GetHeight();
            u32 currentFrame = swapChain->GetCurrentFrame();

            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.pNext = nullptr;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            beginInfo.pInheritanceInfo = nullptr;
            VkCommandBuffer currentbuffer = swapChain->GetCurrentRenderCommandBuffer();
            TBO_VK_ASSERT(vkBeginCommandBuffer(currentbuffer, &beginInfo));

            VkClearValue clear_values[2]{};
            clear_values[0].color = { {0.0f, 0.0f,0.0f, 1.0f} };
            clear_values[1].depthStencil = { 1.0f, 0 };

            VkRenderPassBeginInfo renderPassBeginInfo = {};
            renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassBeginInfo.renderPass = swapChain->GetRenderPass();
            renderPassBeginInfo.renderArea.offset.x = 0;
            renderPassBeginInfo.renderArea.offset.y = 0;
            renderPassBeginInfo.renderArea.extent = { width, height };
            renderPassBeginInfo.clearValueCount = 2; // Color
            renderPassBeginInfo.pClearValues = clear_values;
            renderPassBeginInfo.framebuffer = swapChain->GetCurrentFramebuffer();

            vkCmdBeginRenderPass(currentbuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

            // ImGui secondary command buffer can be recorded pararelly 
            {
                VkCommandBufferInheritanceInfo inheritanceInfo = {};
                inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                inheritanceInfo.renderPass = swapChain->GetRenderPass();
                inheritanceInfo.framebuffer = swapChain->GetCurrentFramebuffer();

                VkCommandBufferBeginInfo cmdBufInfo = {};
                cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
                cmdBufInfo.pInheritanceInfo = &inheritanceInfo;

                TBO_VK_ASSERT(vkBeginCommandBuffer(m_SecondaryBuffers[currentFrame], &cmdBufInfo));
                {
                    VkViewport viewport = {};
                    viewport.x = 0.0f;
                    viewport.y = 0.0f;
                    viewport.width = (f32)width;
                    viewport.height = -(f32)height;
                    viewport.minDepth = 0.0f;
                    viewport.maxDepth = 1.0f;
                    vkCmdSetViewport(m_SecondaryBuffers[currentFrame], 0, 1, &viewport);

                    VkRect2D scissor = {};
                    scissor.extent.width = width;
                    scissor.extent.height = height;
                    scissor.offset.x = 0;
                    scissor.offset.y = 0;
                    vkCmdSetScissor(m_SecondaryBuffers[currentFrame], 0, 1, &scissor);

                    // Record dear imgui primitives into command buffer
                    ImDrawData* main_draw_data = ImGui::GetDrawData();
                    ImGui_ImplVulkan_RenderDrawData(main_draw_data, m_SecondaryBuffers[currentFrame]);
                }
                TBO_VK_ASSERT(vkEndCommandBuffer(m_SecondaryBuffers[currentFrame]));

                // Execute imgui secondary command buffer
                vkCmdExecuteCommands(currentbuffer, 1, &m_SecondaryBuffers[currentFrame]);
            }

            vkCmdEndRenderPass(currentbuffer);

            // End swapchain's primary secondary buffer
            TBO_VK_ASSERT(vkEndCommandBuffer(currentbuffer));
        }

        ImGuiIO& io = ImGui::GetIO();

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }

    void VulkanUserInterface::OnEvent(Event& e)
    {
        if (m_BlockEvents)
        {
            ImGuiIO& io = ImGui::GetIO();
            e.Handled |= e.IsInCategory(EventCategory_Mouse) & io.WantCaptureMouse;
            e.Handled |= e.IsInCategory(EventCategory_Keyboard) & io.WantCaptureKeyboard;
        }
    }

}
