workspace "immg"
    configurations 
    {
        "debug", 
        "release"
    }
    architecture "x64"
    language "C++"

project "immg"
    location "immg"
    kind "ConsoleApp"
    targetdir "%{wks.location}/build/%{cfg.buildcfg}"
    objdir "%{wks.location}/build/obj"
    files 
    {
        "%{prj.location}/src/**.h",
        "%{prj.location}/src/**.cpp",
        "%{wks.location}/thirdparty/glad/src/glad.c"
    }
    -- External libraries setup
    includedirs
    {
        "%{wks.location}/thirdparty/SDL2-2.0.20/include",
        "%{wks.location}/thirdparty/glad/include",
        "%{wks.location}/thirdparty/stb/"
    }
    libdirs
    {
        "%{wks.location}/thirdparty/SDL2-2.0.20/lib/x64"
    }
    
    links
    {
        "SDL2main", "SDL2"
    }
    
    -- Build cofigurations setup
    filter "configurations:debug"
        defines
        {
            "IMMG_DEBUG",
            "_CRT_SECURE_NO_WARNINGS"
        }
        symbols "On"
    
    filter "configurations:release"
        defines
        {
            "IMMG_NDEBUG"
        }
        optimize "On"
