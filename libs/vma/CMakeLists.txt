add_library(vma STATIC
    src/vk_mem_alloc.cpp
)

target_include_directories(vma PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(vma PUBLIC
    Vulkan::Vulkan
)

