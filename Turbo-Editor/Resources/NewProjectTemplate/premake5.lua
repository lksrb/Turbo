TurboRootDirectory = "../../"

workspace "%PROJECT_NAME%"
    startproject "%PROJECT_NAME%"
	configurations {
	    "Debug",
		"Release",
    }

group "Turbo"
project "Turbo-ScriptCore"
    location "%{TurboRootDirectory}Turbo-ScriptCore"
    language "C#"
	kind "SharedLib"
	dotnetframework "4.7.2"

	targetdir ("%{TurboRootDirectory}Turbo-Editor/Resources/Scripts")
	objdir ("%{TurboRootDirectory}Turbo-Editor/Resources/Scripts/Intermediates")

    files {
        "%{TurboRootDirectory}Turbo-ScriptCore/Source/**.cs"
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
project "%PROJECT_NAME%"
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
