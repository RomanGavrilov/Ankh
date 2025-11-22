# src/pipeline/CMakeLists.txt

add_library(ankh_pipeline STATIC
    pipeline-layout.cpp
    graphics-pipeline.cpp
)

target_link_libraries(ankh_pipeline
    PUBLIC
        ankh_utils
        ankh_core
        ankh_renderpass
        ankh_descriptors
        ankh_shaders
)

target_include_directories(ankh_pipeline
    PUBLIC
        ${ANKH_SRC_ROOT}
)
