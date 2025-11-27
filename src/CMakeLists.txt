# src/CMakeLists.txt

set(ANKH_SRC_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(utils)
add_subdirectory(platform)
add_subdirectory(core)
add_subdirectory(swapchain)
add_subdirectory(renderpass)
add_subdirectory(descriptors)
add_subdirectory(pipeline)
add_subdirectory(sync)
add_subdirectory(shaders)
add_subdirectory(renderer)
add_subdirectory(app)
add_subdirectory(memory)
add_subdirectory(commands)
add_subdirectory(frame)

add_executable(Ankh
    main.cpp
)

target_link_libraries(Ankh
    PRIVATE
        ankh_warnings
        ankh_app
)

# ⬇️ Shader compile + copy must be here, after Ankh is created
add_custom_command(
    TARGET Ankh
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -DDESTINATION_DIR=$<TARGET_FILE_DIR:Ankh>/shaders -P "${CMAKE_SOURCE_DIR}/shaders/compile.cmake"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/shaders"
            "$<TARGET_FILE_DIR:Ankh>/shaders"
    COMMENT "Compiling and copying shaders"
)
