# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(InfraredBrickTower)

find_package(imgui CONFIG REQUIRED)

add_subdirectory(IBTController)
add_subdirectory(ConsoleTester)
add_subdirectory(PBrickLogic)
add_subdirectory(TowerLogic)

add_executable(InfraredBrickTower IBTController/IBTController.cpp)

target_link_libraries(InfraredBrickTower PRIVATE imgui::imgui)

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

target_link_libraries(InfraredBrickTower PRIVATE winusb)
target_link_libraries(InfraredBrickTower PRIVATE cfgmgr32)
target_link_libraries(InfraredBrickTower PRIVATE IBTDriverWin)
target_link_libraries(InfraredBrickTower PRIVATE TowerLogic)
target_link_libraries(InfraredBrickTower PRIVATE PBrickLogic)
