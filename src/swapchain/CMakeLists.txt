add_library(ankh_swapchain INTERFACE)


target_include_directories(ankh_swapchain INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)


target_link_libraries(ankh_swapchain INTERFACE Vulkan::Vulkan)