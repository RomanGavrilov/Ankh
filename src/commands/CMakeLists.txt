add_library(ankh_commands STATIC
    command-pool.cpp
    command-buffer.cpp
)

target_include_directories(ankh_commands
    PUBLIC
        ${ANKH_SRC_ROOT}
)

target_link_libraries(ankh_commands
    PUBLIC
        ankh_utils
        Vulkan::Vulkan
)
