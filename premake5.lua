
--Turbo Workspace

include "./dependencies/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"

workspace "Turbo"
    architecture "x86_64"
	startproject "Turbo-Editor"

	solution_items {
		".editorconfig"
	}

	configurations {
	    "Debug",
		"Release",
    }

	flags {
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}";

group ".Dependencies"
	include "dependencies/ImGui"
	include "dependencies/box2d"
	include "dependencies/yaml-cpp"
	include "dependencies/msdf-atlas-gen"
	include "dependencies/optick"
group "Core"
	include "Turbo"
	include "Turbo-ScriptCore"

group "Tools"
	include "Turbo-Editor"
	
