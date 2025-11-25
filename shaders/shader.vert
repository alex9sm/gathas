#version 460

layout(binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
} camera;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;

void main() {
    gl_Position = camera.proj * camera.view * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragNormal = inNormal;
    fragTexCoord = inTexCoord;
}