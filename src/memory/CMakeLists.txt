add_library(ankh_memory STATIC
    buffer.cpp
    upload-context.cpp
)

target_include_directories(ankh_memory
    PUBLIC
        ${ANKH_SRC_ROOT}
)

target_link_libraries(ankh_memory
    PUBLIC
        ankh_utils
        Vulkan::Vulkan
)
