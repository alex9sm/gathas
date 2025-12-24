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
layout(set = 1, binding = 3) uniform sampler2D roughnessSampler;

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

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

vec3 reconstructWorldPosition(float depth, vec2 texCoord) {
    vec4 clipPos = vec4(texCoord.x * 2.0 - 1.0, 1.0 - texCoord.y * 2.0, depth, 1.0);
    vec4 viewPos = camera.invProj * clipPos;
    viewPos /= viewPos.w;
    vec4 worldPos = camera.invView * viewPos;
    return worldPos.xyz;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float distributionGGX(float NdotH, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.14159265359 * denom * denom;

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
    vec3 albedo = texture(albedoSampler, fragTexCoord).rgb;
    vec3 normal = normalize(texture(normalSampler, fragTexCoord).rgb * 2.0 - 1.0);
    float depth = texture(depthSampler, fragTexCoord).r;
    float roughness = max(texture(roughnessSampler, fragTexCoord).r, 0.04);
    if (depth >= 1.0) {
        outColor = vec4(albedo, 1.0);
        return;
    }
    vec3 worldPos = reconstructWorldPosition(depth, fragTexCoord);
    vec3 viewDir = normalize(light.cameraPosition - worldPos);
    vec3 lightDir = normalize(-light.direction);
    vec3 halfDir = normalize(lightDir + viewDir);

    float NdotL = max(dot(normal, lightDir), 0.0);
    float NdotV = max(dot(normal, viewDir), 0.0);
    float NdotH = max(dot(normal, halfDir), 0.0);
    float HdotV = max(dot(halfDir, viewDir), 0.0);

    vec3 ambient = light.ambientColor * light.ambientIntensity * albedo;
    vec3 diffuse = light.color * light.intensity * NdotL * albedo;
    vec3 F0 = vec3(0.04); // default dielectric
    vec3 F = fresnelSchlick(HdotV, F0);
    float D = distributionGGX(NdotH, roughness);
    float G = geometrySmith(NdotV, NdotL, roughness);

    // Cook-Torrance
    float denominator = 4.0 * NdotV * NdotL + 0.0001;
    vec3 specular = (D * F * G) / denominator;
    specular = specular * light.color * light.intensity * NdotL;

    // point light
    vec3 pointLightDir = normalize(pointLight.position - worldPos);
    float distance = length(pointLight.position - worldPos);
    float attenuation = 1.0 / (distance * distance);

    float pNdotL = max(dot(normal, pointLightDir), 0.0);
    vec3 pHalfDir = normalize(pointLightDir + viewDir);
    float pNdotH = max(dot(normal, pHalfDir), 0.0);
    float pHdotV = max(dot(pHalfDir, viewDir), 0.0);

    // point light diffuse
    vec3 pDiffuse = pointLight.color * pointLight.intensity * pNdotL * albedo * attenuation;

    // point light specular
    vec3 pF = fresnelSchlick(pHdotV, F0);
    float pD = distributionGGX(pNdotH, roughness);
    float pG = geometrySmith(NdotV, pNdotL, roughness);
    float pDenom = 4.0 * NdotV * pNdotL + 0.0001;
    vec3 pSpecular = (pD * pF * pG) / pDenom;
    pSpecular = pSpecular * pointLight.color * pointLight.intensity * pNdotL * attenuation;

    // combine lighting
    vec3 finalColor = ambient + diffuse + specular + pDiffuse + pSpecular;

    outColor = vec4(finalColor, 1.0);
}
