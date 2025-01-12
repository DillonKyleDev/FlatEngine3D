project "FlatEngine-Editor"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20"
   targetdir "%{cfg.buildcfg}"
   staticruntime "off"

   files { 
        "Source/**.h",
        "Source/**.cpp",
    }

   includedirs
   {
      "Source",
	  -- Include Core
	  "../FlatEngine-Core/Source",
      "../scripts",
      "../Vendor/includes/ImGui/Backends",
      "../Vendor/includes/ImGui/ImGui_Docking",
      "../Vendor/includes/ImGui/ImGui_Docking/misc/debuggers",
      "../Vendor/includes/SDL2/include",
      "../Vendor/includes/SDL2_Image/include",
      "../Vendor/includes/SDL2_Text/include",
      "../Vendor/includes/SDL2_Mixer/include",
      "../Vendor/includes/Json_Formatter",
      "../Vendor/includes/ImPlot",
      "../Vendor/includes/ImSequencer",
      "../Vendor/includes/Lua",
      "../Vendor/includes/Sol2/include",
      "../Vendor/includes/Sol2/include/sol",
   }

   externalincludedirs 
   {
    "../Vendor/includes/Vulkan/include",
    "../Vendor/includes/GLM/glm",
    "../Vendor/includes/GLFW/include",
     "../Vendor/includes/GLFW/lib-vc2022"
   }

   libdirs 
   {
        "../Vendor/includes/SDL2/lib/x64",
        "../Vendor/includes/SDL2_Image/lib/x64",
        "../Vendor/includes/SDL2_Text/lib/x64",
        "../Vendor/includes/SDL2_Mixer/lib/x64",
        "../Vendor/includes/ImPlot",
        "../Vendor/includes/ImSequencer",
        "../Vendor/includes/Lua",
   }

   links
   {
        "FlatEngine-Core",
        "SDL2.lib",
        "SDL2main.lib",
        "SDL2_ttf.lib",
        "SDL2_image.lib",
        "d3d12.lib",
        "d3dcompiler.lib",
        "dxgi.lib",
        "SDL2_mixer.lib",
        "lua54.lib",        
   }

   defines
   {
    "_WINDOWS"
   }

--    targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
--    objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

    targetdir ("C:/Users/Dillon Kyle/Desktop/FlatEngine/" .. OutputDir .. "/%{prj.name}")
    objdir ("C:/Users/Dillon Kyle/Desktop/FlatEngine/Intermediates/" .. OutputDir .. "/%{prj.name}")

   filter "system:windows"
       systemversion "latest"
       defines { "WINDOWS" }

   filter "configurations:Debug"
       defines { "_DEBUG" }
       runtime "Debug"
       symbols "On"

   filter "configurations:Release"
       defines { "NDEBUG" }
       runtime "Release"
       optimize "On"
       symbols "On"

   filter "configurations:Dist"
       defines { "NDEBUG" }
       runtime "Release"
       optimize "On"
       symbols "Off"