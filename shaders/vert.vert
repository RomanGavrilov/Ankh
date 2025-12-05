#version 450

layout(binding = 0) uniform FrameUBO {
    mat4 view;
    mat4 proj;
    vec4 globalAlbedo;    // not used in VS yet, but available
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

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;
layout(location = 2) out vec4 fragAlbedo; // pass per-object albedo to FS

void main() {
    ObjectData obj = objects[pc.objectIndex];

    gl_Position = frame.proj * frame.view * obj.model * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
    fragUV = inUV;
    fragAlbedo = obj.albedo;
}
