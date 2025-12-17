#version 460
#extension GL_KHR_vulkan_glsl : enable

layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outNormal;

void main() {
    outAlbedo = texture(texSampler, fragTexCoord);

    vec3 normal = normalize(fragNormal);
    outNormal = vec4(normal * 0.5 + 0.5, 1.0);
}
