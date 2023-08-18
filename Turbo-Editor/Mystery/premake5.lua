
workspace "Mystery"
    startproject "Mystery"
	configurations {
	    "Debug",
		"Release",
    }

group "Turbo"
project "Turbo-ScriptCore"
    location "C:/dev/.cpp/Turbo/Turbo-ScriptCore"
    language "C#"
	kind "SharedLib"
	dotnetframework "4.7.2"

	targetdir ("C:/dev/.cpp/Turbo/Turbo-Editor/Resources/Scripts")
	objdir ("C:/dev/.cpp/Turbo/Turbo-Editor/Resources/Scripts/Intermediates")

    files {
        "C:/dev/.cpp/Turbo/Turbo-ScriptCore/Source/**.cs"
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
project "Mystery"
    location "Assets/Scripts"
    language "C#"
	kind "SharedLib"
	dotnetframework "4.7.2"

	targetdir ("%{wks.location}/Binaries")
	objdir ("%{wks.location}/Binaries/Intermediates")

    files {
        "Assets/Scripts/**.cs"
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
