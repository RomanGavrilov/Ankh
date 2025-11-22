add_library(ankh_core INTERFACE)


target_include_directories(ankh_core INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)


target_link_libraries(ankh_core INTERFACE Vulkan::Vulkan)