file(GLOB SOURCE_H
        "Config.h"
        "HostTowerCommInterface.h"
        "TowerController.h"
        "pch.h")

file(GLOB SOURCE_C
        "HostTowerCommInterface.cpp"
        "TowerController.cpp"
        "pch.cpp")

add_library(TowerLogic ${SOURCE_C})