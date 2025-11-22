add_library(ankh_renderer INTERFACE)


target_include_directories(ankh_renderer INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)


target_link_libraries(ankh_renderer INTERFACE Vulkan::Vulkan)