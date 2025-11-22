# src/renderpass/CMakeLists.txt

add_library(ankh_renderpass STATIC
    render-pass.cpp
    frame-buffer.cpp
)

target_link_libraries(ankh_renderpass
    PUBLIC
        ankh_utils
        ankh_core
)

target_include_directories(ankh_renderpass
    PUBLIC
        ${ANKH_SRC_ROOT}
)
