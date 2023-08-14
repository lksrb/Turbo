
-- Turbo Engine

project "Turbo"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
	staticruntime "off"
	floatingpoint "Fast"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-obj/" .. outputdir .. "/%{prj.name}")

    pchheader "tbopch.h"
	pchsource "src/tbopch.cpp"

    files {
        "src/**.h",
        "src/**.cpp",

        "dependencies/ImGuizmo/**.h",
        "dependencies/ImGuizmo/**.cpp",

        "dependencies/IconFontCppHeaders/**.h",
        "dependencies/IconFontCppHeaders/**.cpp"
    }

    defines {
        "_CRT_SECURE_NO_WARNINGS",
	    "YAML_CPP_STATIC_DEFINE"
    }

    includedirs {
        "src",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.msdfgen}",
        "%{IncludeDir.msdf_atlas_gen}",
        "%{IncludeDir.VulkanSDK}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.ImGuizmo}",
        "%{IncludeDir.IconFontCppHeaders}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.stb}",
        "%{IncludeDir.box2d}",
        "%{IncludeDir.JoltPhysics}",
        "%{IncludeDir.yaml_cpp}",
        "%{IncludeDir.mono}",
        "%{IncludeDir.Assimp}"
    }

    links {
        "ImGui",
        "box2d",
        "yaml-cpp",
        "msdf-atlas-gen",
        "%{Library.Vulkan}",
        "%{Library.mono}"
    }

    -- Ignore already defined symbols warning(LNK4006), symbols not found(LNK4099)
	linkoptions {
		"-IGNORE:4006",
        "-IGNORE:4099"
	}
    
    --filter "files:dependencies/ImGuizmo/**.cpp"
      --  flags { "NoPCH" }

	filter "system:windows"
        systemversion "latest"

        defines {
            "TBO_PLATFORM_WIN32"
        }

        links {
            "%{Library.WinSock}",
            "%{Library.WinMM}",
            "%{Library.WinVersion}",
            "%{Library.WinBcryp}",
            "XAudio2",
            "Shlwapi.lib"
        }

    filter "configurations:Debug"
        defines "TBO_DEBUG"
        runtime "Debug"
        symbols "on"

        links
        {
           "%{Library.ShaderC_Debug}",
           "%{Library.SPIRV_Cross_Debug}",
           "%{Library.SPIRV_Cross_GLSL_Debug}",
           "%{Library.Assimp_Debug}",
           "%{Library.JoltPhysics_Debug}" -- Maybe link with release for performance
        }

    filter "configurations:Release"
        defines "TBO_RELEASE"
        runtime "Release"
        optimize "on"

        links
        {
            "%{Library.ShaderC_Release}",
            "%{Library.SPIRV_Cross_Release}",
            "%{Library.SPIRV_Cross_GLSL_Release}",
            "%{Library.Assimp_Release}",
            "%{Library.JoltPhysics_Release}"
        }

