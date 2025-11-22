# src/shaders/CMakeLists.txt

add_library(ankh_shaders STATIC
    shader-module.cpp
)

target_link_libraries(ankh_shaders
    PUBLIC
        ankh_utils      # for utils/file-io.hpp and types.hpp
        ankh_core       # for VkDevice / Vulkan linkage
)

target_include_directories(ankh_shaders
    PUBLIC
        ${ANKH_SRC_ROOT}
)
