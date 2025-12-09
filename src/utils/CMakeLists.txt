# src/utils/CMakeLists.txt

add_library(ankh_utils STATIC
    types.cpp
    file-io.cpp
    logging.cpp
    config.cpp
    stb-image-impl.cpp
    stb-image-write-impl.cpp
    tiny-gltf-impl.cpp
)

target_include_directories(ankh_utils
    PUBLIC
        ${ANKH_SRC_ROOT}
        ${CMAKE_SOURCE_DIR}/libs/tinygltf
        ${CMAKE_SOURCE_DIR}/libs/stb
        ${CMAKE_SOURCE_DIR}/libs/json
)

target_link_libraries(ankh_utils
    PUBLIC
        glm     # header-only, but OK to link
        glfw
        Vulkan::Vulkan
)
