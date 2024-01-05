file(GLOB SOURCE_H
        "Types.h"
        "Utilities.h")

file(GLOB SOURCE_C
        "Utilities.cpp")

add_library(Common ${SOURCE_C})
