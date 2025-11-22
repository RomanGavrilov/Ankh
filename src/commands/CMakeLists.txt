add_library(ankh_commands INTERFACE)


target_include_directories(ankh_commands INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)


target_link_libraries(ankh_commands INTERFACE Vulkan::Vulkan)