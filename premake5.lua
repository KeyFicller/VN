envir_work_space_directory = os.getcwd()

envir_output_directory = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

envir_include_directory = envir_work_space_directory .. "/include"
envir_client_directory = envir_work_space_directory .. "/client"
envir_server_directory = envir_work_space_directory .. "/server"
envir_third_party_directory = envir_work_space_directory .. "/third"

workspace "VisualNurb"
	startproject "client"
	configurations {
		"Debug",
		"Release"
	}
	filter "system:windows"
		platforms {
			"x64"
		}

	filter "configurations: Debug"
		symbols "On"
	filter "configurations: Release"
		symbols "On"


include "client"
include "server"
include "premake"
include "third"