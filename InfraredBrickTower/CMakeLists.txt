file(GLOB SOURCE_H
        "WinUsbTowerInterface.h"
        "pch.h")

file(GLOB SOURCE_C
        "WinUsbTowerInterface.cpp")

add_library(InfraredBrickTower ${SOURCE_C})