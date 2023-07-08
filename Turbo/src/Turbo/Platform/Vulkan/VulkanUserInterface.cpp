#include "tbopch.h"
#include "VulkanUserInterface.h"

#include "Turbo/Core/Engine.h"
#include "Turbo/Core/Platform.h"

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
#pragma region ColorTweaks
    namespace
    { // Unnamed namespace, since we only use this here. 
        unsigned int Color(unsigned int c)
        {
            // add alpha.
            // also swap red and blue channel for some reason.
            // todo: figure out why, and fix it.
            const short a = 0xFF;
            const short r = (c >> 16) & 0xFF;
            const short g = (c >> 8) & 0xFF;
            const short b = (c >> 0) & 0xFF;
            return(a << 24)
                | (r << 0)
                | (g << 8)
                | (b << 16);
        }
    }

    namespace Spectrum
    {
#ifdef LIGHT
        const unsigned int GRAY50 = Color(0xFFFFFF);
        const unsigned int GRAY75 = Color(0xFAFAFA);
        const unsigned int GRAY100 = Color(0xF5F5F5);
        const unsigned int GRAY200 = Color(0xEAEAEA);
        const unsigned int GRAY300 = Color(0xE1E1E1);
        const unsigned int GRAY400 = Color(0xCACACA);
        const unsigned int GRAY500 = Color(0xB3B3B3);
        const unsigned int GRAY600 = Color(0x8E8E8E);
        const unsigned int GRAY700 = Color(0x707070);
        const unsigned int GRAY800 = Color(0x4B4B4B);
        const unsigned int GRAY900 = Color(0x2C2C2C);
        const unsigned int BLUE400 = Color(0x2680EB);
        const unsigned int BLUE500 = Color(0x1473E6);
        const unsigned int BLUE600 = Color(0x0D66D0);
        const unsigned int BLUE700 = Color(0x095ABA);
        const unsigned int RED400 = Color(0xE34850);
        const unsigned int RED500 = Color(0xD7373F);
        const unsigned int RED600 = Color(0xC9252D);
        const unsigned int RED700 = Color(0xBB121A);
        const unsigned int ORANGE400 = Color(0xE68619);
        const unsigned int ORANGE500 = Color(0xDA7B11);
        const unsigned int ORANGE600 = Color(0xCB6F10);
        const unsigned int ORANGE700 = Color(0xBD640D);
        const unsigned int GREEN400 = Color(0x2D9D78);
        const unsigned int GREEN500 = Color(0x268E6C);
        const unsigned int GREEN600 = Color(0x12805C);
        const unsigned int GREEN700 = Color(0x107154);
        const unsigned int INDIGO400 = Color(0x6767EC);
        const unsigned int INDIGO500 = Color(0x5C5CE0);
        const unsigned int INDIGO600 = Color(0x5151D3);
        const unsigned int INDIGO700 = Color(0x4646C6);
        const unsigned int CELERY400 = Color(0x44B556);
        const unsigned int CELERY500 = Color(0x3DA74E);
        const unsigned int CELERY600 = Color(0x379947);
        const unsigned int CELERY700 = Color(0x318B40);
        const unsigned int MAGENTA400 = Color(0xD83790);
        const unsigned int MAGENTA500 = Color(0xCE2783);
        const unsigned int MAGENTA600 = Color(0xBC1C74);
        const unsigned int MAGENTA700 = Color(0xAE0E66);
        const unsigned int YELLOW400 = Color(0xDFBF00);
        const unsigned int YELLOW500 = Color(0xD2B200);
        const unsigned int YELLOW600 = Color(0xC4A600);
        const unsigned int YELLOW700 = Color(0xB79900);
        const unsigned int FUCHSIA400 = Color(0xC038CC);
        const unsigned int FUCHSIA500 = Color(0xB130BD);
        const unsigned int FUCHSIA600 = Color(0xA228AD);
        const unsigned int FUCHSIA700 = Color(0x93219E);
        const unsigned int SEAFOAM400 = Color(0x1B959A);
        const unsigned int SEAFOAM500 = Color(0x16878C);
        const unsigned int SEAFOAM600 = Color(0x0F797D);
        const unsigned int SEAFOAM700 = Color(0x096C6F);
        const unsigned int CHARTREUSE400 = Color(0x85D044);
        const unsigned int CHARTREUSE500 = Color(0x7CC33F);
        const unsigned int CHARTREUSE600 = Color(0x73B53A);
        const unsigned int CHARTREUSE700 = Color(0x6AA834);
        const unsigned int PURPLE400 = Color(0x9256D9);
        const unsigned int PURPLE500 = Color(0x864CCC);
        const unsigned int PURPLE600 = Color(0x7A42BF);
        const unsigned int PURPLE700 = Color(0x6F38B1);

#else // Dark
        const unsigned int GRAY50 = Color(0x252525);
        const unsigned int GRAY75 = Color(0x2F2F2F);
        const unsigned int GRAY100 = Color(0x323232);
        const unsigned int GRAY200 = Color(0x393939);
        const unsigned int GRAY300 = Color(0x3E3E3E);
        const unsigned int GRAY400 = Color(0x4D4D4D);
        const unsigned int GRAY500 = Color(0x5C5C5C);
        const unsigned int GRAY600 = Color(0x7B7B7B);
        const unsigned int GRAY700 = Color(0x999999);
        const unsigned int GRAY800 = Color(0xCDCDCD);
        const unsigned int GRAY900 = Color(0xFFFFFF);
        const unsigned int BLUE400 = Color(0x2680EB);
        const unsigned int BLUE500 = Color(0x378EF0);
        const unsigned int BLUE600 = Color(0x4B9CF5);
        const unsigned int BLUE700 = Color(0x5AA9FA);
        const unsigned int RED400 = Color(0xE34850);
        const unsigned int RED500 = Color(0xEC5B62);
        const unsigned int RED600 = Color(0xF76D74);
        const unsigned int RED700 = Color(0xFF7B82);
        const unsigned int ORANGE400 = Color(0xE68619);
        const unsigned int ORANGE500 = Color(0xF29423);
        const unsigned int ORANGE600 = Color(0xF9A43F);
        const unsigned int ORANGE700 = Color(0xFFB55B);
        const unsigned int GREEN400 = Color(0x2D9D78);
        const unsigned int GREEN500 = Color(0x33AB84);
        const unsigned int GREEN600 = Color(0x39B990);
        const unsigned int GREEN700 = Color(0x3FC89C);
        const unsigned int INDIGO400 = Color(0x6767EC);
        const unsigned int INDIGO500 = Color(0x7575F1);
        const unsigned int INDIGO600 = Color(0x8282F6);
        const unsigned int INDIGO700 = Color(0x9090FA);
        const unsigned int CELERY400 = Color(0x44B556);
        const unsigned int CELERY500 = Color(0x4BC35F);
        const unsigned int CELERY600 = Color(0x51D267);
        const unsigned int CELERY700 = Color(0x58E06F);
        const unsigned int MAGENTA400 = Color(0xD83790);
        const unsigned int MAGENTA500 = Color(0xE2499D);
        const unsigned int MAGENTA600 = Color(0xEC5AAA);
        const unsigned int MAGENTA700 = Color(0xF56BB7);
        const unsigned int YELLOW400 = Color(0xDFBF00);
        const unsigned int YELLOW500 = Color(0xEDCC00);
        const unsigned int YELLOW600 = Color(0xFAD900);
        const unsigned int YELLOW700 = Color(0xFFE22E);
        const unsigned int FUCHSIA400 = Color(0xC038CC);
        const unsigned int FUCHSIA500 = Color(0xCF3EDC);
        const unsigned int FUCHSIA600 = Color(0xD951E5);
        const unsigned int FUCHSIA700 = Color(0xE366EF);
        const unsigned int SEAFOAM400 = Color(0x1B959A);
        const unsigned int SEAFOAM500 = Color(0x20A3A8);
        const unsigned int SEAFOAM600 = Color(0x23B2B8);
        const unsigned int SEAFOAM700 = Color(0x26C0C7);
        const unsigned int CHARTREUSE400 = Color(0x85D044);
        const unsigned int CHARTREUSE500 = Color(0x8EDE49);
        const unsigned int CHARTREUSE600 = Color(0x9BEC54);
        const unsigned int CHARTREUSE700 = Color(0xA3F858);
        const unsigned int PURPLE400 = Color(0x9256D9);
        const unsigned int PURPLE500 = Color(0x9D64E1);
        const unsigned int PURPLE600 = Color(0xA873E9);
        const unsigned int PURPLE700 = Color(0xB483F0);
#endif
    }

    void VulkanUserInterface::ImGuiStyleSpectrum()
    {
        using namespace ImGui;

        ImGuiStyle* style = &ImGui::GetStyle();
        style->GrabRounding = 4.0f;
        ImVec4* colors = style->Colors;
        colors[ImGuiCol_Text] = ColorConvertU32ToFloat4(Spectrum::GRAY800); // text on hovered controls is gray900
        colors[ImGuiCol_TextDisabled] = ColorConvertU32ToFloat4(Spectrum::GRAY500);
        colors[ImGuiCol_WindowBg] = ColorConvertU32ToFloat4(Spectrum::GRAY100);
        colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_PopupBg] = ColorConvertU32ToFloat4(Spectrum::GRAY50); // not sure about this. Note: applies to tooltips too.
        colors[ImGuiCol_Border] = ColorConvertU32ToFloat4(Spectrum::GRAY300);
        colors[ImGuiCol_BorderShadow] = ColorConvertU32ToFloat4(0x00000000); // We don't want shadows. Ever.
        colors[ImGuiCol_FrameBg] = ColorConvertU32ToFloat4(Spectrum::GRAY75); // this isnt right, spectrum does not do this, but it's a good fallback
        colors[ImGuiCol_FrameBgHovered] = ColorConvertU32ToFloat4(Spectrum::GRAY50);
        colors[ImGuiCol_FrameBgActive] = ColorConvertU32ToFloat4(Spectrum::GRAY200);
        colors[ImGuiCol_TitleBg] = ColorConvertU32ToFloat4(Spectrum::GRAY300); // those titlebar values are totally made up, spectrum does not have this.
        colors[ImGuiCol_TitleBgActive] = ColorConvertU32ToFloat4(Spectrum::GRAY200);
        colors[ImGuiCol_TitleBgCollapsed] = ColorConvertU32ToFloat4(Spectrum::GRAY400);
        colors[ImGuiCol_MenuBarBg] = ColorConvertU32ToFloat4(Spectrum::GRAY100);
        colors[ImGuiCol_ScrollbarBg] = ColorConvertU32ToFloat4(Spectrum::GRAY100); // same as regular background
        colors[ImGuiCol_ScrollbarGrab] = ColorConvertU32ToFloat4(Spectrum::GRAY400);
        colors[ImGuiCol_ScrollbarGrabHovered] = ColorConvertU32ToFloat4(Spectrum::GRAY600);
        colors[ImGuiCol_ScrollbarGrabActive] = ColorConvertU32ToFloat4(Spectrum::GRAY700);
        colors[ImGuiCol_CheckMark] = ColorConvertU32ToFloat4(Spectrum::BLUE500);
        colors[ImGuiCol_SliderGrab] = ColorConvertU32ToFloat4(Spectrum::GRAY700);
        colors[ImGuiCol_SliderGrabActive] = ColorConvertU32ToFloat4(Spectrum::GRAY800);
        colors[ImGuiCol_Button] = ColorConvertU32ToFloat4(Spectrum::GRAY75); // match default button to Spectrum's 'Action Button'.
        colors[ImGuiCol_ButtonHovered] = ColorConvertU32ToFloat4(Spectrum::GRAY50);
        colors[ImGuiCol_ButtonActive] = ColorConvertU32ToFloat4(Spectrum::GRAY200);
        colors[ImGuiCol_Header] = ColorConvertU32ToFloat4(Spectrum::BLUE400);
        colors[ImGuiCol_HeaderHovered] = ColorConvertU32ToFloat4(Spectrum::BLUE500);
        colors[ImGuiCol_HeaderActive] = ColorConvertU32ToFloat4(Spectrum::BLUE600);
        colors[ImGuiCol_Separator] = ColorConvertU32ToFloat4(Spectrum::GRAY400);
        colors[ImGuiCol_SeparatorHovered] = ColorConvertU32ToFloat4(Spectrum::GRAY600);
        colors[ImGuiCol_SeparatorActive] = ColorConvertU32ToFloat4(Spectrum::GRAY700);
        colors[ImGuiCol_ResizeGrip] = ColorConvertU32ToFloat4(Spectrum::GRAY400);
        colors[ImGuiCol_ResizeGripHovered] = ColorConvertU32ToFloat4(Spectrum::GRAY600);
        colors[ImGuiCol_ResizeGripActive] = ColorConvertU32ToFloat4(Spectrum::GRAY700);
        colors[ImGuiCol_PlotLines] = ColorConvertU32ToFloat4(Spectrum::BLUE400);
        colors[ImGuiCol_PlotLinesHovered] = ColorConvertU32ToFloat4(Spectrum::BLUE600);
        colors[ImGuiCol_PlotHistogram] = ColorConvertU32ToFloat4(Spectrum::BLUE400);
        colors[ImGuiCol_PlotHistogramHovered] = ColorConvertU32ToFloat4(Spectrum::BLUE600);
        colors[ImGuiCol_TextSelectedBg] = ColorConvertU32ToFloat4((Spectrum::BLUE400 & 0x00FFFFFF) | 0x33000000);
        colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        colors[ImGuiCol_NavHighlight] = ColorConvertU32ToFloat4((Spectrum::GRAY900 & 0x00FFFFFF) | 0x0A000000);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
    }
#pragma endregion

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
                case Turbo::Hidden: return ImGuiMouseCursor_None;
                case Turbo::Arrow: return ImGuiMouseCursor_Arrow;
                case Turbo::Hand: return ImGuiMouseCursor_Hand;
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
        io.Fonts->AddFontFromFileTTF("Assets\\Fonts\\IconFonts\\" FONT_ICON_FILE_NAME_FAR, 16.0f, &icons_config, icons_ranges);
        //io.FontGlobalScale = 1.5f;

        ImGuiStyleSpectrum();

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
        m_SecondaryBuffers.resize(framesInFlight);
        RendererContext::CreateSecondaryCommandBuffers(m_SecondaryBuffers.data(), framesInFlight);
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
