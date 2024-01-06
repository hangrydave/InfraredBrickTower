file(GLOB SOURCE_H
        "Config.h"
        "HostTowerCommInterface.h"
        "WinUsbTowerInterface.h"
        "LinuxUsbTowerInterface.h"
        "TowerController.h"
        "pch.h")

file(GLOB SOURCE_C
        "HostTowerCommInterface.cpp"
        "WinUsbTowerInterface.cpp"
        "LinuxUsbTowerInterface.cpp"
        "TowerController.cpp"
        "pch.cpp")

add_library(TowerLogic ${SOURCE_C})

if (WIN32)
        target_include_directories(TowerLogic PUBLIC ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/IBTDriverWin)

        add_library(IBTDriverWin STATIC IMPORTED)
        set_target_properties(IBTDriverWin PROPERTIES
                IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/x64/Debug/IBTDriverWin.lib"
                INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/IBTDriverWin"
        )

        target_link_libraries(TowerLogic PUBLIC IBTDriverWin)
else()
        target_link_libraries(TowerLogic PRIVATE PkgConfig::libusb)
endif()
