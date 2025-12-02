#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

// Same UBO layout as in vertex shader
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 albedo;
} ubo;

// Binding 1: texture
layout(binding = 1) uniform sampler2D uTexture;

void main() {
    const vec2 resolution = vec2(800.0, 600.0);
    vec2 uv = gl_FragCoord.xy / resolution;

    vec4 texColor = texture(uTexture, uv);

    // Tint texture * vertex color * material albedo
    vec4 base = texColor * vec4(fragColor, 1.0);
    outColor = base * ubo.albedo;
}
