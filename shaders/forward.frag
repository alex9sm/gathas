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

layout(set = 3, binding = 0) uniform PointLightUBO {
    vec3 position;
    float intensity;
    vec3 color;
    float padding;
} pointLight;

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

layout(location = 0) out vec4 outColor;

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float distributionGGX(float NdotH, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.1416 * denom * denom;

    return num / denom;
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float geometrySmith(float NdotV, float NdotL, float roughness) {
    float ggx1 = geometrySchlickGGX(NdotV, roughness);
    float ggx2 = geometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

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

    vec3 halfDir = normalize(lightDir + viewDir);

    // dot products
    float NdotL = max(dot(normal, lightDir), 0.0);
    float NdotV = max(dot(normal, viewDir), 0.0);
    float NdotH = max(dot(normal, halfDir), 0.0);
    float HdotV = max(dot(halfDir, viewDir), 0.0);

    // ambient
    vec3 ambient = light.ambientColor * light.ambientIntensity * albedo.rgb;
    vec3 diffuse = light.color * light.intensity * NdotL * albedo.rgb;

    // specular
    float roughness = max(material.roughness, 0.04);
    vec3 F0 = vec3(0.04); // default dielectric F0
    vec3 F = fresnelSchlick(HdotV, F0);
    float D = distributionGGX(NdotH, roughness);
    float G = geometrySmith(NdotV, NdotL, roughness);

    // Cook-Torrance
    float denominator = 4.0 * NdotV * NdotL + 0.0001;
    vec3 specular = (D * F * G) / denominator;
    specular = specular * light.color * light.intensity * NdotL;

    // point light contribution
    vec3 pointLightDir = normalize(pointLight.position - fragWorldPos);
    float distance = length(pointLight.position - fragWorldPos);
    float attenuation = 1.0 / (distance * distance);

    float pNdotL = max(dot(normal, pointLightDir), 0.0);
    vec3 pHalfDir = normalize(pointLightDir + viewDir);
    float pNdotH = max(dot(normal, pHalfDir), 0.0);
    float pHdotV = max(dot(pHalfDir, viewDir), 0.0);

    // point light diffuse
    vec3 pDiffuse = pointLight.color * pointLight.intensity * pNdotL * albedo.rgb * attenuation;

    // point light specular
    vec3 pF = fresnelSchlick(pHdotV, F0);
    float pD = distributionGGX(pNdotH, roughness);
    float pG = geometrySmith(NdotV, pNdotL, roughness);
    float pDenom = 4.0 * NdotV * pNdotL + 0.0001;
    vec3 pSpecular = (pD * pF * pG) / pDenom;
    pSpecular = pSpecular * pointLight.color * pointLight.intensity * pNdotL * attenuation;

    vec3 finalColor = ambient + diffuse + specular + pDiffuse + pSpecular;

    outColor = vec4(finalColor, alpha);
}
