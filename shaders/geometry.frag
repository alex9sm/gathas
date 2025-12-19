#version 460
#extension GL_KHR_vulkan_glsl : enable

layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(push_constant) uniform MaterialPushConstants {
    vec4 diffuseColor;
    uint hasTexture;
} material;

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outNormal;

void main() {
    // use constant color if no texture, otherwise sample texture
    if (material.hasTexture == 1) {
        outAlbedo = texture(texSampler, fragTexCoord);
    } else {
        outAlbedo = vec4(material.diffuseColor.rgb, 1.0);
    }
    vec3 normal = normalize(fragNormal);
    outNormal = vec4(normal * 0.5 + 0.5, 1.0);
}
