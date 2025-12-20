#version 460
#extension GL_KHR_vulkan_glsl : enable

layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
    mat4 invView;
    mat4 invProj;
} camera;

layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 1) uniform sampler2D normalMapSampler;

layout(set = 2, binding = 0) uniform LightingUBO {
    vec3 direction;
    float intensity;
    vec3 color;
    float ambientIntensity;
    vec3 ambientColor;
    float specularPower;
    vec3 cameraPosition;
    float padding;
} light;

layout(push_constant) uniform MaterialPushConstants {
    vec4 diffuseColor;
    uint hasTexture;
    uint hasNormalMap;
    float dissolve;
    float padding;
} material;

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec4 fragTangent;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 albedo;
    if (material.hasTexture == 1) {
        albedo = texture(texSampler, fragTexCoord);
    } else {
        albedo = vec4(material.diffuseColor.rgb, 1.0);
    }

    float alpha = albedo.a * material.dissolve;

    if (alpha < 0.01) {
        discard;
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

    vec3 viewDir = normalize(light.cameraPosition - fragWorldPos);

    vec3 lightDir = normalize(-light.direction);

    vec3 ambient = light.ambientColor * light.ambientIntensity * albedo.rgb;

    float NdotL = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.color * light.intensity * NdotL * albedo.rgb;

    vec3 halfDir = normalize(lightDir + viewDir);
    float NdotH = max(dot(normal, halfDir), 0.0);
    float specularStrength = pow(NdotH, light.specularPower);
    vec3 specular = light.color * light.intensity * specularStrength * 0.5;

    vec3 finalColor = ambient + diffuse + specular;

    outColor = vec4(finalColor, alpha);
}
