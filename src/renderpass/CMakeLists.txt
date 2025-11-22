add_library(ankh_renderpass INTERFACE)


target_include_directories(ankh_renderpass INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)


target_link_libraries(ankh_renderpass INTERFACE Vulkan::Vulkan)