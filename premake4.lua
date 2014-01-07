solution "samples"
	configurations { "Debug", "Release" }
	location "build"
	libdirs {"$(DXSDK_DIR)Lib/x86"}
	includedirs {"$(DXSDK_DIR)Include/"}	
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
	
	project "simple"
		kind "WindowedApp"
		language "C++"
		files 
		{ 
			"src/d3du.cpp",
			"src/d3du.h",
			"src/math.h",
			"src/util.h",
			"src/util.cpp",
			"src/simple.cpp"
		}
