#version 450

layout(binding = 0) uniform FrameUBO {
    mat4 view;
    mat4 proj;
    vec4 globalAlbedo;
    vec4 lightDir;
} frame;

struct ObjectData {
    mat4 model;
    vec4 albedo;
};

layout(std430, binding = 1) readonly buffer ObjectBuffer {
    ObjectData objects[];
};

layout(push_constant) uniform ObjectPC {
    uint objectIndex;
} pc;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;
layout(location = 2) out vec4 fragAlbedo;
layout(location = 3) out vec3 fragNormal; // world-space

void main() {
    ObjectData obj = objects[pc.objectIndex];

    mat4 model = obj.model;

    gl_Position = frame.proj * frame.view * model * vec4(inPosition, 1.0);

    // Transform normal to world space
    // If you want performance:
    // Precompute normal matrix CPU-side and store it in ObjectDataGPU
    mat3 normalMat = transpose(inverse(mat3(model)));
    fragNormal = normalize(normalMat * inNormal);

    fragColor  = inColor;
    fragUV     = inUV;
    fragAlbedo = obj.albedo;
}
