project "server"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir(envir_work_space_directory .. "/bin/" .. envir_output_directory .. "/%{prj.name}")
    objdir(envir_work_space_directory .. "/bin-int/" .. envir_output_directory .. "/%{prj.name}")

    defines {
        "VN_PROJECT",
        "USE_GLM_IN_MODULE"
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
        envir_third_party_directory .. "/CDT/CDT/include"
    }

    postbuildcommands {
        --("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. envir_output_directory .. "/sandbox/\""),
    }

    links {
        "glfw",
        "opengl32.lib",
        "glu32.lib",
        "glad"
    }