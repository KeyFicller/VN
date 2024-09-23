project "rebuild"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"

    targetdir(envir_output_directory .. "/bin/" .. envir_output_directory .. "/%{prj.name}")
    objdir(envir_output_directory .. "/bin-int/" .. envir_output_directory .. "/%{prj.name}")

    postbuildcommands {
        "cd ..",
        "call build.bat"
    }