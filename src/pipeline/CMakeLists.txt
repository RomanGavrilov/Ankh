add_library(ankh_pipeline INTERFACE)


target_include_directories(ankh_pipeline INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)


target_link_libraries(ankh_pipeline INTERFACE Vulkan::Vulkan)