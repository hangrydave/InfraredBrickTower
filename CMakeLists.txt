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
pkg_check_modules(libusb REQUIRED IMPORTED_TARGET libusb-1.0)

add_subdirectory(IBTController)
add_subdirectory(ConsoleTester)

add_subdirectory(PBrickLogic)
add_subdirectory(TowerLogic)

include_directories("${CMAKE_SOURCE_DIR}/PBrickLogic")
include_directories("${CMAKE_SOURCE_DIR}/TowerLogic")

if (WIN32)
else()
	add_subdirectory(IBTDriverLinux)
	include_directories("${CMAKE_SOURCE_DIR}/IBTDriverLinux")
endif()

include_directories("${CMAKE_SOURCE_DIR}/lib/imgui_filebrowser/imgui-filebrowser")

add_executable(
	InfraredBrickTower
	IBTController/IBTController.cpp
	IBTController/ControllerUI.h
	IBTController/ControllerUI.cpp)

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

	add_library(IBTDriverWin STATIC IMPORTED)
	set_target_properties(IBTDriverWin PROPERTIES
		IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/x64/Debug/IBTDriverWin.lib"
		INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/IBTDriverWin"
	)
else()
endif()

#set(IMGUI_FILEBROWSER_SRC 
#	${CMAKE_SOURCE_DIR}/lib/imgui_filebrowser/imgui-filebrowser/imfilebrowser.h)

#target_sources(InfraredBrickTower PRIVATE ${IMGUI_FILEBROWSER_SRC})

target_link_libraries(InfraredBrickTower PRIVATE glfw)
target_link_libraries(InfraredBrickTower PRIVATE TowerLogic)
target_link_libraries(InfraredBrickTower PRIVATE PBrickLogic)

if (WIN32)
	# https://github.com/microsoft/DirectXTK12/wiki/Adding-the-DirectX-Tool-Kit
	# target_link_libraries(InfraredBrickTower PRIVATE
	# 	d3d12.lib dxgi.lib dxguid.lib uuid.lib
	# 	kernel32.lib user32.lib
	# 	comdlg32.lib advapi32.lib shell32.lib
	# 	ole32.lib oleaut32.lib
	# 	runtimeobject.lib
	# 	Microsoft::DirectXTK12
	# )
	target_link_libraries(InfraredBrickTower PRIVATE winusb)
	target_link_libraries(InfraredBrickTower PRIVATE cfgmgr32)
	target_link_libraries(InfraredBrickTower PRIVATE IBTDriverWin)
	target_link_libraries(InfraredBrickTower PRIVATE opengl32.lib)
else()
	target_link_libraries(InfraredBrickTower PRIVATE PkgConfig::libusb)
	target_link_libraries(InfraredBrickTower PRIVATE IBTDriverLinux)
	target_link_libraries(InfraredBrickTower PRIVATE OpenGL)
endif()
