solution "samples"
	configurations { "Debug", "Release" }
	location "build"
	libdirs {"$(DXSDK_DIR)Lib/x86/", "extern/assimp/lib/", "extern/stb/lib/", "extern/lua/src/", "bin"}
	includedirs {"$(DXSDK_DIR)Include/", "extern/assimp/include/", "extern/jq/", "extern/stb/include/", "extern/lua/src/", "src/base/"}	
	targetdir "bin"
	debugdir "."
	language "C++"
		
	configuration "Debug"
		defines { "DEBUG" }
		flags { "Symbols" }

	configuration "Release"
		defines { "NDEBUG" }
		flags { "Optimize" }

	project "base"
		kind "StaticLib"
		files
		{
			"src/base/**.cpp",
			"src/base/**.h"
		}

	project "momentous"
		kind "WindowedApp"
		links {"base"}
		files 
		{ 
			"src/d3du.cpp",
			"src/d3du.h",
			"src/main.cpp", 			
		}
	
	project "simpledx"
		kind "WindowedApp"
		links {"d3dx11", "stb", "base", "lua51s"}
		files 
		{ 
			"src/d3du.cpp",
			"src/d3du.h",
			"src/simpledx.cpp"
		}

	project "simplegl"
		kind "WindowedApp"
		includedirs {"extern/glfw/include/", "extern/glew/include/"}
		libdirs	{"extern/glfw/lib/", "extern/glew/lib/Win32/" }
		links { "glfw3dll", "OpenGL32", "glew32", "base" }
		files
		{
			"src/simplegl.cpp"
		}

	project "0_test"
		kind "ConsoleApp"
		links {"base"}
		files
		{ "src/unittest.cpp"}