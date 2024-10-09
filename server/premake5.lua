project "client"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir(envir_work_space_directory .. "/bin/" .. envir_output_directory .. "/%{prj.name}")
    objdir(envir_work_space_directory .. "/bin-int/" .. envir_output_directory .. "/%{prj.name}")

    open_nurbs_path_relative = envir_third_party_directory .. "/opennurbs"
    open_nurbs_path_absolute = path.getabsolute(open_nurbs_path_relative)
    defines {
        "VN_PROJECT",
        "USE_GLM_IN_MODULE",
        "USE_IMGUI_IN_MODULE",
        "YAML_CPP_STATIC_DEFINE",
        "OPENNURBS_PUBLIC_INSTALL_DIR=" .. open_nurbs_path_absolute
    }

    files {
        envir_include_directory .. "/**.h",
        "inc/**.h",
        "src/**.cpp",
        "premake5.lua"
    }

    ignoredefaultlibraries {
        "LIBCMTD"
    }

    vpaths {
        ["include/*"] = { envir_include_directory .. "/**.h"}
    }

    includedirs {
        "inc",
        envir_include_directory,
        envir_third_party_directory .. "/glfw/include",
        envir_third_party_directory .. "/glad/include",
        envir_third_party_directory .. "/glm",
        envir_third_party_directory .. "/CDT/CDT/include",
        envir_third_party_directory .. "/yaml-cpp/include",
        envir_third_party_directory .. "/imgui",
        open_nurbs_path_absolute
    }

    postbuildcommands {
        --("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. envir_output_directory .. "/sandbox/\""),
    }

    links {
        "glfw",
        "opengl32.lib",
        "glu32.lib",
        "glad",
        "yaml-cpp",
        "imgui"
    }