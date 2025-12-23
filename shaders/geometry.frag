#version 460
#extension GL_KHR_vulkan_glsl : enable

layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 1) uniform sampler2D normalMapSampler;

layout(push_constant) uniform MaterialPushConstants {
    vec4 diffuseColor;
    uint hasTexture;
    uint hasNormalMap;
    float dissolve;
    float roughness;
} material;

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec4 fragTangent;

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out float outRoughness;

void main() {
    // use constant color if no texture, otherwise sample texture
    if (material.hasTexture == 1) {
        outAlbedo = texture(texSampler, fragTexCoord);
    } else {
        outAlbedo = vec4(material.diffuseColor.rgb, 1.0);
    }

    vec3 normal;
    if (material.hasNormalMap == 1) {
        vec3 N = normalize(fragNormal);
        vec3 T = normalize(fragTangent.xyz);
        T = normalize(T - dot(T, N) * N);
        vec3 B = cross(N, T) * fragTangent.w;
        mat3 TBN = mat3(T, B, N);
        vec3 sampledNormal = texture(normalMapSampler, fragTexCoord).rgb;
        sampledNormal = sampledNormal * 2.0 - 1.0;
        normal = normalize(TBN * sampledNormal);
    } else {
        normal = normalize(fragNormal);
    }

    outNormal = vec4(normal * 0.5 + 0.5, 1.0);
    outRoughness = material.roughness;
}
