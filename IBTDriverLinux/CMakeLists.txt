file(GLOB SOURCE_H "LinuxUsbTowerInterface.h")
file(GLOB SOURCE_C "LinuxUsbTowerInterface.cpp")

add_library(IBTDriverLinux ${SOURCE_C})

target_link_libraries(IBTDriverLinux PUBLIC TowerLogic)
target_link_libraries(IBTDriverLinux PRIVATE PkgConfig::libusb)
target_include_directories(IBTDriverLinux PUBLIC ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/TowerLogic)
