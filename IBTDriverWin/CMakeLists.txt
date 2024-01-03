file(GLOB SOURCE_H "WinUsbTowerInterface.h")
file(GLOB SOURCE_C "WinUsbTowerInterface.cpp")

add_library(IBTDriverWin ${SOURCE_C})

target_link_libraries(IBTDriverWin PUBLIC TowerLogic)
target_include_directories(IBTDriverWin PUBLIC ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/TowerLogic)