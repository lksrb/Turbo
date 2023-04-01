
--Turbo Dependencies

IncludeDir= {}
IncludeDir["Turbo"] = "%{wks.location}/Turbo/src"
IncludeDir["GLFW"] = "%{wks.location}/dependencies/GLFW/include"
IncludeDir["spdlog"] = "%{wks.location}/dependencies/spdlog/include"
IncludeDir["ImGui"] = "%{wks.location}/dependencies/imgui"
IncludeDir["ImGuizmo"] = "%{wks.location}/dependencies/ImGuizmo"
IncludeDir["glm"] = "%{wks.location}/dependencies/glm"
IncludeDir["mono"] = "%{wks.location}/dependencies/mono/include"
IncludeDir["stb"] = "%{wks.location}/dependencies/stb"
IncludeDir["yaml_cpp"] = "%{wks.location}/dependencies/yaml-cpp/include"
IncludeDir["entt"] = "%{wks.location}/dependencies/entt"
IncludeDir["box2d"] = "%{wks.location}/dependencies/Box2D/include"
IncludeDir["IconFontCppHeaders"] = "%{wks.location}/dependencies/IconFontCppHeaders"
IncludeDir["msdfgen"] = "%{wks.location}/dependencies/msdf-atlas-gen/msdfgen"
IncludeDir["msdf_atlas_gen"] = "%{wks.location}/dependencies/msdf-atlas-gen/msdf-atlas-gen"

-- Vulkan
VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir["shaderc"] =					"%{wks.location}/dependencies/shaderc/include"
IncludeDir["SPIRV_Cross"] =				"%{wks.location}/dependencies/SPIRV-Cross"
IncludeDir["VulkanSDK"] =				"%{VULKAN_SDK}/Include"

LibraryDir = {}
LibraryDir["monodir"] =					"%{wks.location}/dependencies/mono/lib/%{cfg.buildcfg}"

LibraryDir["VulkanSDK"] =				"%{VULKAN_SDK}/Lib"
LibraryDir["VulkanSDK_Debug"] =			"%{VULKAN_SDK}/Lib"
LibraryDir["VulkanSDK_DebugDLL"] =		"%{VULKAN_SDK}/Bin"

-- Mono

Library = {}
Library["mono"] = "%{LibraryDir.monodir}/libmono-static-sgen.lib"

Library["Vulkan"] =						"%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] =				"%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

Library["ShaderC_Debug"] =				"%{LibraryDir.VulkanSDK_Debug}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] =			"%{LibraryDir.VulkanSDK_Debug}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] =		"%{LibraryDir.VulkanSDK_Debug}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] =			"%{LibraryDir.VulkanSDK_Debug}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] =			"%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] =		"%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] =	"%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"

-- XAudio2
Library["XAudio2"] = "xaudio2.lib"

--Windows
Library["WinSock"] = "Ws2_32.lib"
Library["WinMM"] = "Winmm.lib"
Library["WinVersion"] = "Version.lib"
Library["WinBcryp"] = "Bcrypt.lib"