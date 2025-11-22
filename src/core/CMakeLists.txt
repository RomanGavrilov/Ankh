# src/core/CMakeLists.txt

add_library(ankh_core STATIC
    instance.cpp
    debug-messenger.cpp
    physical-device.cpp
    device.cpp
)

target_link_libraries(ankh_core
    PUBLIC
        ankh_utils
        Vulkan::Vulkan
)

target_include_directories(ankh_core
    PUBLIC
        ${ANKH_SRC_ROOT}
)
