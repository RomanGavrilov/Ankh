# src/utils/CMakeLists.txt

add_library(ankh_utils STATIC
    types.cpp
    file-io.cpp
    logging.cpp
    config.cpp
    gpu-resource-tracker.cpp
    gpu-retirement-queue.cpp
)

target_include_directories(ankh_utils
    PUBLIC
        ${ANKH_SRC_ROOT}
        ${CMAKE_SOURCE_DIR}/libs/tinygltf
        ${CMAKE_SOURCE_DIR}/libs/stb
        ${CMAKE_SOURCE_DIR}/libs/json
        ${CMAKE_SOURCE_DIR}/libs/vma
)

target_link_libraries(ankh_utils
    PUBLIC
        glm
        glfw
        Vulkan::Vulkan
        stb
        nlohmann_json
        tinygltf
)
