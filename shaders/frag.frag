#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;   // ⬅️ from vertex shader

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 albedo;
} ubo;

layout(binding = 1) uniform sampler2D uTexture;

void main() {
    // Use mesh-provided UVs instead of screen-based coords
    vec4 texColor = texture(uTexture, fragUV);

    vec4 base = texColor * vec4(fragColor, 1.0);
    outColor = base * ubo.albedo;
}
