#version 460
#extension GL_KHR_vulkan_glsl : enable

layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
    mat4 invView;
    mat4 invProj;
} camera;

layout(set = 1, binding = 0) uniform sampler2D albedoSampler;
layout(set = 1, binding = 1) uniform sampler2D normalSampler;
layout(set = 1, binding = 2) uniform sampler2D depthSampler;

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    // sample gbuffer
    vec3 albedo = texture(albedoSampler, fragTexCoord).rgb;

    // unpack normal
    vec3 normal = texture(normalSampler, fragTexCoord).rgb * 2.0 - 1.0;

    // sample depth
    float depth = texture(depthSampler, fragTexCoord).r;

    // TODO: lighting calculations will go here
    // reconstruct world position from depth using inverse matrices
    // calculate lighting contribution from directional/point lights
    // apply ambient, diffuse, specular

    // outputs only albedo for now
    outColor = vec4(albedo, 1.0);
}
