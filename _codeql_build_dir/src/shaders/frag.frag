#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec4 fragAlbedo;
layout(location = 3) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

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

layout(binding = 2) uniform sampler2D uTexture;

void main() {
    ObjectData obj = objects[pc.objectIndex];

    vec4 texColor = texture(uTexture, fragUV);
    vec4 base = texColor * vec4(fragColor, 1.0) * obj.albedo;

    vec3 N = normalize(fragNormal);
    vec3 L = normalize(-frame.lightDir.xyz); // lightDir points FROM light

    float NdotL = max(dot(N, L), 0.0);

    float ambient = 0.2;
    float diffuse = 0.8 * NdotL;

    vec3 lit = base.rgb * (ambient + diffuse);

    outColor = vec4(lit, base.a);
}
