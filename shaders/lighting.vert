#version 460
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) out vec2 fragTexCoord;

void main() {
    vec2 position;
    if (gl_VertexIndex == 0) {
        position = vec2(-1.0, -1.0);
    } else if (gl_VertexIndex == 1) {
        position = vec2(3.0, -1.0);
    } else {
        position = vec2(-1.0, 3.0);
    }

    fragTexCoord = position * 0.5 + 0.5;
    fragTexCoord.y = 1.0 - fragTexCoord.y;

    gl_Position = vec4(position, 0.0, 1.0);
}
