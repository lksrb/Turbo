
-- Turbo Script Core

project "Turbo-ScriptCore"
    language "C#"
	kind "SharedLib"
	dotnetframework "4.7.2"

	targetdir ("%{wks.location}/Turbo-Editor/Resources/Scripts")
	objdir ("%{wks.location}/Turbo-Editor/Resources/Scripts/Intermediates")

    files {
        "Source/**.cs"
    }

    filter "system:windows"
        defines "TBO_PLATFORM_WIN32"

    filter "configurations:Debug"
        optimize "Off"
        symbols "Default"

    filter "configurations:Release"
        optimize "On"
        symbols "Default"