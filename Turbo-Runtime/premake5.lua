
--Turbo Editor

project "Turbo-Runtime"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-obj/" .. outputdir .. "/%{prj.name}")

	files 
	{
		"src/**.h",
		"src/**.cpp",
	}
	includedirs 
	{
		"%{wks.location}/Turbo/src",
		"%{wks.location}/dependencies/spdlog/include",
		"%{IncludeDir.glm}"
	}
	links 
	{
		"Turbo"
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
				"{COPYDIR} \"%{LibraryDir.VulkanSDK_DebugDLL}\" \"%{cfg.targetdir}\""
			}

		filter "configurations:Release"
			defines "TBO_RELEASE"
			runtime "Release"
			optimize "on"

			postbuildcommands
			{
				"{COPYDIR} \"%{LibraryDir.VulkanSDK}\" \"%{cfg.targetdir}\""
			}
			