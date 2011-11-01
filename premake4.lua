solution "TemplateGL"
	configurations {
--	"Debug"
	"Release"
	}
	platforms { "x64", "x32" }

-- ---------------------------------------------------------
-- Project 
	project "demoGL"
		basedir "./"
		language "C++"
		location "./"
		kind "ConsoleApp" -- Shouldn't this be in configuration section ?
		files { "*.hpp", "*.cpp" }
		flags "Optimize"
		includedirs {
		"include"
		}
		objdir "obj"
-- Linux x86 platform gmake
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
			
-- Linux x64 platform gmake
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
--		configuration {"Release", "vs*", "x32"}
--			libdirs {
--			"shared/lib/windows/win32"
--			}
--			links {
--			"glew32s",
--			"freeglut",
--			}
--			objdir "obj"
---- Visual x64
--		configuration {"Release", "vs*", "x64"}
--			links {
--			"glew32s",
--			"freeglut",
--			}
--			libdirs {
--			"shared/lib/windows/win64"
--			}
--			objdir "obj"


