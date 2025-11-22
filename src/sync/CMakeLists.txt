add_library(ankh_sync INTERFACE)


target_include_directories(ankh_sync INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)


target_link_libraries(ankh_sync INTERFACE Vulkan::Vulkan)