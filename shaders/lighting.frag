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

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

vec3 reconstructWorldPosition(float depth, vec2 texCoord) {
    vec4 clipPos = vec4(texCoord * 2.0 - 1.0, depth, 1.0);
    vec4 viewPos = camera.invProj * clipPos;
    viewPos /= viewPos.w;
    vec4 worldPos = camera.invView * viewPos;
    return worldPos.xyz;
}

void main() {
    // sample gbuffer
    vec3 albedo = texture(albedoSampler, fragTexCoord).rgb;

    // unpack normal from [0,1] to [-1,1] range
    vec3 normal = normalize(texture(normalSampler, fragTexCoord).rgb * 2.0 - 1.0);

    // sample depth
    float depth = texture(depthSampler, fragTexCoord).r;

    // skip sky pixels (depth == 1.0)
    if (depth >= 1.0) {
        outColor = vec4(albedo, 1.0);
        return;
    }

    // reconstruct world position from depth
    vec3 worldPos = reconstructWorldPosition(depth, fragTexCoord);

    // calculate view direction
    vec3 viewDir = normalize(light.cameraPosition - worldPos);

    // light direction (pointing towards light, so negate the direction)
    vec3 lightDir = normalize(-light.direction);

    // ambient component
    vec3 ambient = light.ambientColor * light.ambientIntensity * albedo;

    // diffuse component (Lambertian)
    float NdotL = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.color * light.intensity * NdotL * albedo;

    // specular component (Blinn-Phong)
    vec3 halfDir = normalize(lightDir + viewDir);
    float NdotH = max(dot(normal, halfDir), 0.0);
    float specularStrength = pow(NdotH, light.specularPower);
    vec3 specular = light.color * light.intensity * specularStrength * 0.5;

    // combine lighting
    vec3 finalColor = ambient + diffuse + specular;

    outColor = vec4(finalColor, 1.0);
}
