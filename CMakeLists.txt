cmake_minimum_required(VERSION 3.10)
project(InfraredBrickTower)

find_package(Vulkan REQUIRED FATAL_ERROR)
find_package(glfw3 REQUIRED FATAL_ERROR)

include_directories(${GLFW_DIR}/deps)

add_subdirectory(TowerLogic)
add_subdirectory(PBrickLogic)
add_subdirectory(InfraredBrickTower)

add_subdirectory(lib/imgui)

add_executable(IBTController
        IBTController/main.cpp
)

target_link_libraries(IBTController PUBLIC
        Vulkan::Vulkan
        glfw
        imgui)

target_include_directories(IBTController PUBLIC ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/lib/imgui/imgui)

target_link_libraries(IBTController PUBLIC TowerLogic)
target_include_directories(IBTController PUBLIC ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/TowerLogic)
target_link_libraries(IBTController PUBLIC PBrickLogic)
target_include_directories(IBTController PUBLIC ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/PBrickLogic)
target_link_libraries(IBTController PUBLIC InfraredBrickTower)
target_include_directories(IBTController PUBLIC ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/InfraredBrickTower)

target_include_directories(IBTController PUBLIC lib/imgui-filebrowser)