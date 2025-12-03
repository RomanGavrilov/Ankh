# src/swapchain/CMakeLists.txt

add_library(ankh_swapchain STATIC
    swapchain.cpp
)

target_link_libraries(ankh_swapchain
    PUBLIC
        ankh_utils
        ankh_core
        ankh_memory
)

target_include_directories(ankh_swapchain
    PUBLIC
        ${ANKH_SRC_ROOT}
)
