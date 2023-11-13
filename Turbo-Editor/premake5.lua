
--Turbo Editor

project "Turbo-Editor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	floatingpoint "Fast"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-obj/" .. outputdir .. "/%{prj.name}")

	files 
	{
		"src/**.h",
		"src/**.cpp",
	}
	includedirs 
	{
		"%{IncludeDir.Turbo}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.imgui_node_editor}",
		"%{IncludeDir.IconFontCppHeaders}",
		"%{IncludeDir.entt}",
	}
	links 
	{
		"Turbo",
		"ImGui"
	}

	-- Ignore already defined symbols warning(LNK4006), symbols not found(LNK4099)
	linkoptions {
		"-IGNORE:4006",
        "-IGNORE:4099"
	}

	filter "system:windows"
		systemversion "latest"

		defines {
            "TBO_PLATFORM_WIN32"
        }

		filter "configurations:Debug"
			defines "TBO_DEBUG"
			runtime "Debug"
			symbols "on"

			postbuildcommands
			{
				-- Apparently this is not necessary but I doubt that when releasing an executable like that
				--"{COPYDIR} \"%{LibraryDir.VulkanSDK_DebugDLL}\" \"%{cfg.targetdir}\"",
				'{COPY} "%{Binaries.Assimp_Debug}" "%{cfg.targetdir}"'
			}
			
		filter "configurations:Release"
			defines "TBO_RELEASE"
			runtime "Release"
			optimize "on"
			
			postbuildcommands
			{
				--"{COPYDIR} \"%{LibraryDir.VulkanSDK}\" \"%{cfg.targetdir}\"",
				'{COPY} "%{Binaries.Assimp_Release}" "%{cfg.targetdir}"'
			}
			