add_library(vma STATIC
    src/vk-mem-alloc.cpp
)

target_include_directories(vma PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(vma PUBLIC
    Vulkan::Vulkan
)

# Use loader-linked Vulkan functions (best for engines)
target_compile_definitions(vma PUBLIC
    VMA_STATIC_VULKAN_FUNCTIONS=0
    VMA_DYNAMIC_VULKAN_FUNCTIONS=0
)
