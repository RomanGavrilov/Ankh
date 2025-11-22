add_library(ankh_descriptors INTERFACE)


target_include_directories(ankh_descriptors INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)


target_link_libraries(ankh_descriptors INTERFACE Vulkan::Vulkan)