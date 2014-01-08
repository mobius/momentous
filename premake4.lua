solution "samples"
	configurations { "Debug", "Release" }
	location "build"
	libdirs {"$(DXSDK_DIR)Lib/x86/", "extern/assimp/lib/"}
	includedirs {"$(DXSDK_DIR)Include/", "extern/assimp/include/"}	
	targetdir "bin"
	debugdir "."
		
	configuration "Debug"
		defines { "DEBUG" }
		flags { "Symbols" }

	configuration "Release"
		defines { "NDEBUG" }
		flags { "Optimize" }

	project "momentous"
		kind "WindowedApp"
		language "C++"
		files 
		{ 
			"src/d3du.cpp",
			"src/d3du.h",
			"src/math.h",
			"src/util.h",
			"src/util.cpp",
			"src/main.cpp", 			
		}
	
	project "simpledx"
		kind "WindowedApp"
		language "C++"
		files 
		{ 
			"src/d3du.cpp",
			"src/d3du.h",
			"src/math.h",
			"src/util.h",
			"src/util.cpp",
			"src/simpledx.cpp"
		}

	project "simplegl"
		kind "WindowedApp"
		language "C++"
		includedirs {"extern/glfw/include/", "extern/glew/include/"}
		libdirs	{"extern/glfw/lib/", "extern/glew/lib/Win32/" }
		links { "glfw3dll", "OpenGL32", "glew32" }
		files
		{
			"src/simplegl.cpp"
		}