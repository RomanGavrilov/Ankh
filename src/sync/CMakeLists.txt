# src/sync/CMakeLists.txt

add_library(ankh_sync STATIC
    sync-primitives.cpp
)

target_link_libraries(ankh_sync
    PUBLIC
        ankh_utils
        ankh_core
)

target_include_directories(ankh_sync
    PUBLIC
        ${ANKH_SRC_ROOT}
)
