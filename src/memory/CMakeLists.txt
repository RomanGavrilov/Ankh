add_library(ankh_memory INTERFACE)


target_include_directories(ankh_memory INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)


target_link_libraries(ankh_memory INTERFACE Vulkan::Vulkan)