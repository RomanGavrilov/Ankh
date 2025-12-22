# src/renderer/CMakeLists.txt

add_library(ankh_renderer STATIC
    renderer.cpp
    draw-pass.cpp
    ui-pass.cpp
    scene-renderer.cpp
    gpu-mesh-pool.cpp
)

target_link_libraries(ankh_renderer
    PUBLIC
        ankh_core
        ankh_utils
        ankh_platform
        ankh_swapchain
        ankh_renderpass
        ankh_descriptors
        ankh_pipeline
        ankh_sync
        ankh_commands
        ankh_frame
        ankh_scene
        ankh_streaming
)

target_include_directories(ankh_renderer
    PUBLIC
        ${ANKH_SRC_ROOT}
)
