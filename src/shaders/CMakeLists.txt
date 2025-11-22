add_library(ankh_shaders INTERFACE)


target_include_directories(ankh_shaders INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)


target_link_libraries(ankh_shaders INTERFACE Vulkan::Vulkan)