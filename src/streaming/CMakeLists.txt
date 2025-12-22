# src/scene/CMakeLists.txt

add_library(ankh_streaming STATIC
  async-uploader.cpp
)

target_include_directories(ankh_streaming
    PUBLIC
        ${ANKH_SRC_ROOT}
)

target_link_libraries(ankh_streaming
    PUBLIC
        ankh_utils
)