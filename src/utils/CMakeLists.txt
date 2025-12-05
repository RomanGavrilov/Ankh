# src/utils/CMakeLists.txt

add_library(ankh_utils STATIC
    types.cpp
    file-io.cpp
    logging.cpp
    config.cpp
)

target_include_directories(ankh_utils
    PUBLIC
        ${ANKH_SRC_ROOT}
)

target_link_libraries(ankh_utils
    PUBLIC
        glm     # header-only, but OK to link
        glfw
        Vulkan::Vulkan
)
