# src/scene/CMakeLists.txt

add_library(ankh_scene STATIC
    camera.cpp
    # mesh.cpp
    # material.cpp
)

target_include_directories(ankh_scene
    PUBLIC
        ${ANKH_SRC_ROOT}
)

target_link_libraries(ankh_scene
    PUBLIC
        ankh_utils
)