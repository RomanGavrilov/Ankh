# src/app/CMakeLists.txt

add_library(ankh_app STATIC
    application.cpp
)

target_link_libraries(ankh_app
    PUBLIC
        ankh_renderer
)

target_include_directories(ankh_app
    PUBLIC
        ${ANKH_SRC_ROOT}
)
