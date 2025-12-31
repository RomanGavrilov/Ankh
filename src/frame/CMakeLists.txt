add_library(ankh_frame STATIC
    frame-context.cpp
    frame-allocator.cpp
    frame-allocator-tests.cpp
)

target_include_directories(ankh_frame
    PUBLIC
        ${ANKH_SRC_ROOT}
)

target_link_libraries(ankh_frame
    PUBLIC
        ankh_utils
        ankh_commands
        ankh_memory
        ankh_descriptors
        Vulkan::Vulkan
)
