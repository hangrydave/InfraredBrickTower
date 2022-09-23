file(GLOB SOURCE_H
        "imgui/imgui.h"
        "imgui/backends/imgui_impl_vulkan.h"
        "imgui/backends/imgui_impl_glfw.h")

file(GLOB SOURCE_C
        imgui/imgui.cpp
        imgui/backends/imgui_impl_vulkan.cpp
        imgui/backends/imgui_impl_glfw.cpp
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp)

add_library(imgui ${SOURCE_C})

target_include_directories(imgui PUBLIC imgui)