
local TurboRootDirectory = "../../"

workspace "SandboxProject"
    startproject "SandboxProject"
	configurations {
	    "Debug",
		"Release",
    }

group "Turbo"
project "Turbo-ScriptCore"
    location "../../Turbo-ScriptCore"
    language "C#"
	kind "SharedLib"
	dotnetframework "4.7.2"

	targetdir ("../../Turbo-Editor/Resources/Scripts")
	objdir ("../../Turbo-Editor/Resources/Scripts/Intermediates")

    files {
        "../../Turbo-ScriptCore/Source/**.cs"
    }

    filter "system:windows"
        defines "TBO_PLATFORM_WIN32"

    filter "configurations:Debug"
        optimize "Off"
        symbols "Default"

    filter "configurations:Release"
        optimize "On"
        symbols "Default"

group ""
project "SandboxProject"
    location "Assets/Scripts"
    language "C#"
	kind "SharedLib"
	dotnetframework "4.7.2"

	targetdir ("%{wks.location}/Binaries")
	objdir ("%{wks.location}/Binaries/Intermediates")

    files {
        "Assets/Scripts/Source/**.cs"
    }

    links {
        "Turbo-ScriptCore"
    }

    filter "configurations:Debug"
        optimize "Off"
        symbols "Default"

    filter "configurations:Release"
        optimize "On"
        symbols "Default"
