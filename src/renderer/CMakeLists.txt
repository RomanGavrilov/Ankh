# src/renderer/CMakeLists.txt

add_library(ankh_renderer STATIC
    renderer.cpp
)

target_link_libraries(ankh_renderer
    PUBLIC
        ankh_utils
        ankh_platform
        ankh_core
        ankh_swapchain
        ankh_renderpass
        ankh_descriptors
        ankh_pipeline
        ankh_sync
        ankh_memory
        ankh_commands
        
)

target_include_directories(ankh_renderer
    PUBLIC
        ${ANKH_SRC_ROOT}
)
