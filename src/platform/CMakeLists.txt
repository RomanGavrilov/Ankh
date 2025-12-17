# src/platform/CMakeLists.txt

add_library(ankh_platform STATIC
    window.cpp
    surface.cpp
)

target_link_libraries(ankh_platform
    PUBLIC
        ankh_utils
        glfw
)

target_include_directories(ankh_platform
    PUBLIC
        ${ANKH_SRC_ROOT}
)
