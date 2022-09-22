file(GLOB SOURCE_H
        "Config.h"
        "HostTowerCommInterface.h"
        "LinuxUSBTowerInterface.h"
        "TowerController.cpp"
        "pch.h")

file(GLOB SOURCE_C
        "HostTowerCommInterface.cpp"
        "LinuxUSBTowerInterface.cpp"
        "TowerController.cpp"
        "pch.cpp")

add_library(TowerLogic ${SOURCE_C})