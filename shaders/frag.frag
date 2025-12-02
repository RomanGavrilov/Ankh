#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

// Binding 1: combined image sampler (matches descriptor set layout)
layout(binding = 1) uniform sampler2D uTexture;

void main() {
    // Very simple UV from screen coordinates.
    // Uses your fixed window size; good enough for a demo.
    const vec2 resolution = vec2(800.0, 600.0);
    vec2 uv = gl_FragCoord.xy / resolution;

    vec4 texColor = texture(uTexture, uv);
    outColor = texColor * vec4(fragColor, 1.0);
}
