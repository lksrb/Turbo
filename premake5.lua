
--Turbo Workspace

include "./dependencies/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"

workspace "Turbo"
    architecture "x86_64"
	startproject "PacManGame"

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
	include "dependencies/lua"
	include "dependencies/box2d"
group "Core"
	include "Turbo"

group "Core-Tools"
	include "Turbo-Editor"
	
	--group "Runtime"
	--	include "Turbo-Runtime"
group "Game"
	include "PacManGame"
