file(GLOB SOURCE_H
        "LASM.h"
        "PBrick.h"
        "VLL.h"
        "pch.h")

file(GLOB SOURCE_C
        "LASM.cpp"
        "PBrick.cpp"
        "VLL.cpp"
        "pch.cpp")

add_library(PBrickLogic ${SOURCE_C})

target_link_libraries(PBrickLogic PUBLIC TowerLogic)
target_include_directories(PBrickLogic PUBLIC ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/TowerLogic)
