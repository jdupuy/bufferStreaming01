solution "TemplateGL"
	configurations {
	"Debug"
--	"Release"
	}
	platforms { "x64", "x32" }

-- ---------------------------------------------------------
-- Project 
	project "demoGL"
		basedir "./"
		language "C++"
		location "./"
		kind "ConsoleApp"
		files { "*.hpp", "*.cpp" }
		flags "Symbols"
		includedirs {
		"include"
		}
		objdir "obj"
-- Linux x86
		configuration {"linux", "gmake", "x32"}
			linkoptions {
			"-Wl,-rpath -Wl,./lib/linux/lin32",
			}
			libdirs {
			"lib/linux/lin32"
			}
			links {
			"GLEW",
			"glut",
			"AntTweakBar"
			}

-- Linux x64
		configuration {"linux", "gmake", "x64"}
			linkoptions {
			"-Wl,-rpath -Wl,./lib/linux/lin64"
			}
			libdirs {
			"lib/linux/lin64"
			}
			links {
			"GLEW",
			"glut",
			"AntTweakBar"
			}

-- Visual x86
		configuration {"vs2010", "x32"}
			libdirs {
			"lib/windows/win32"
			}
			links {
			"glew32s",
			"freeglut",
			}
			objdir "obj"

---- Visual x64
--		configuration {"vs2010", "x64"}
--			links {
--			"glew32s",
--			"freeglut",
--			}
--			libdirs {
--			"lib/windows/win64"
--			}
--			objdir "obj"


