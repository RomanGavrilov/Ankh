# Create interface libraries per module for now (headers only). Later we'll
# turn these into STATIC libs with sources when you say "Implement".


add_subdirectory(utils)
add_subdirectory(platform)
add_subdirectory(core)
add_subdirectory(swapchain)
add_subdirectory(renderpass)
add_subdirectory(pipeline)
add_subdirectory(descriptors)
add_subdirectory(memory)
add_subdirectory(commands)
add_subdirectory(sync)
add_subdirectory(shaders)
add_subdirectory(frame)
add_subdirectory(renderer)
add_subdirectory(scene)
add_subdirectory(app)


# Executable — can be your existing main.cpp or the stub in app/
add_executable(Ankh
app/Main.cpp
)


# Public include root for headers within src/
target_include_directories(Ankh PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


target_link_libraries(Ankh PRIVATE
ankh_warnings
Vulkan::Vulkan
glfw
glm
ankh_utils
ankh_platform
ankh_core
ankh_swapchain
ankh_renderpass
ankh_pipeline
ankh_descriptors
ankh_memory
ankh_commands
ankh_sync
ankh_shaders
ankh_frame
ankh_renderer
ankh_scene
ankh_app
)


# Require C++17 at the target level
target_compile_features(Ankh PRIVATE cxx_std_17)