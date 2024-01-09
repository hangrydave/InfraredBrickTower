# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

if (WIN32)
	# Enable Hot Reload for MSVC compilers if supported.
	if (POLICY CMP0141)
		cmake_policy(SET CMP0141 NEW)
		set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
	endif()
endif()

project(InfraredBrickTower)

## Find dependencies
find_package(glfw3 REQUIRED)

# Use  cmake -S . -B build/ -DOpenGL_GL_PREFERENCE=GLVND
# to skip legacy OpenGL library
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
find_package(imgui REQUIRED)
find_package(PkgConfig REQUIRED)

IF (WIN32)
else()
	pkg_check_modules(libusb REQUIRED IMPORTED_TARGET libusb-1.0)
endif()

add_subdirectory(IBTController)
add_subdirectory(ConsoleTester)

add_subdirectory(Common)
add_subdirectory(PBrickLogic)
add_subdirectory(TowerLogic)

include_directories("${CMAKE_SOURCE_DIR}/Common")
include_directories("${CMAKE_SOURCE_DIR}/PBrickLogic")
include_directories("${CMAKE_SOURCE_DIR}/TowerLogic")

include_directories("${CMAKE_SOURCE_DIR}/lib/imgui_filebrowser/imgui-filebrowser")

add_executable(
	InfraredBrickTower
	IBTController/main.cpp
	IBTController/ControllerUI.h
	IBTController/ControllerUI.cpp)

add_executable(
	IBTConsoleTester
	ConsoleTester/main.cpp)

add_executable(
	ibt
	IBTConsole/main.cpp)

target_link_libraries(InfraredBrickTower PRIVATE imgui::imgui)

if (WIN32)
	add_library(winusb STATIC IMPORTED)
	set_target_properties(winusb PROPERTIES
		IMPORTED_LOCATION "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22621.0/um/x64/winusb.lib"
		INTERFACE_INCLUDE_DIRECTORIES "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/um"
	)

	add_library(cfgmgr32 STATIC IMPORTED)
	set_target_properties(cfgmgr32 PROPERTIES
		IMPORTED_LOCATION "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22621.0/um/x64/cfgmgr32.lib"
		INTERFACE_INCLUDE_DIRECTORIES "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/um"
	)
endif()

target_link_libraries(InfraredBrickTower PRIVATE glfw)
target_link_libraries(InfraredBrickTower PRIVATE Common)
target_link_libraries(InfraredBrickTower PRIVATE TowerLogic)
target_link_libraries(InfraredBrickTower PRIVATE PBrickLogic)

target_link_libraries(IBTConsoleTester PRIVATE Common)
target_link_libraries(IBTConsoleTester PRIVATE TowerLogic)
target_link_libraries(IBTConsoleTester PRIVATE PBrickLogic)

target_link_libraries(ibt PRIVATE Common)
target_link_libraries(ibt PRIVATE TowerLogic)
target_link_libraries(ibt PRIVATE PBrickLogic)

if (WIN32)
	target_link_libraries(InfraredBrickTower PRIVATE winusb)
	target_link_libraries(InfraredBrickTower PRIVATE cfgmgr32)
	target_link_libraries(InfraredBrickTower PRIVATE opengl32.lib)
	
	target_link_libraries(IBTConsoleTester PRIVATE winusb)
	target_link_libraries(IBTConsoleTester PRIVATE cfgmgr32)

	target_link_libraries(ibt PRIVATE winusb)
	target_link_libraries(ibt PRIVATE cfgmgr32)
else()
	target_link_libraries(InfraredBrickTower PRIVATE PkgConfig::libusb)
	target_link_libraries(InfraredBrickTower PRIVATE OpenGL)

	target_link_libraries(IBTConsoleTester PRIVATE PkgConfig::libusb)

	target_link_libraries(ibt PRIVATE PkgConfig::libusb)
endif()
