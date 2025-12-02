# src/descriptors/CMakeLists.txt

add_library(ankh_descriptors STATIC
    descriptor-set-layout.cpp
    descriptor-pool.cpp
    descriptor-writer.cpp
)

target_link_libraries(ankh_descriptors
    PUBLIC
        ankh_utils
        ankh_core
)

target_include_directories(ankh_descriptors
    PUBLIC
        ${ANKH_SRC_ROOT}
)
