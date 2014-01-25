solution "stb"
	configurations { "Debug", "Release" }
	location "build"
	includedirs {"include/"}	
	targetdir "lib"

	configuration "Debug"
		defines { "DEBUG" }
		flags { "Symbols" }
		
	configuration "Release"
		defines { "NDEBUG" }
		flags { "Optimize", "Symbols" }

	project "stb"
		kind "StaticLib"
		language "C"
		files {"src/**.c"}

