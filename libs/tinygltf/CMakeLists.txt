add_library(tinygltf STATIC
    src/tiny-gltf.cpp
)

target_include_directories(tinygltf PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# tinygltf's headers include json + stb headers => PUBLIC deps
target_link_libraries(tinygltf PUBLIC
    nlohmann_json
    stb
)

# Optional but recommended for clean builds on MSVC:
target_compile_definitions(tinygltf PRIVATE
    _CRT_SECURE_NO_WARNINGS
)
