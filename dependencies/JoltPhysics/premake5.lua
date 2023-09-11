
-- Jolt Physics

project "JoltPhysics"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
	staticruntime "off"
	floatingpoint "Fast"
    
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-obj/" .. outputdir .. "/%{prj.name}")

    files {
        "Jolt/**.h",
        "Jolt/**.cpp"
    }

    includedirs {
        "."
    }

    excludes  { 
        "Jolt/Renderer/**.h",
        "Jolt/Renderer/**.cpp",
    }

	filter "system:windows"
        systemversion "latest"

        defines {
            "WIN32",
            "_WINDOWS"
        }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

        defines {
            "_DEBUG"
        }

    filter "configurations:Release"
        runtime "Release"
        optimize "on"
        
        defines {
            "NDEBUG"
        }
