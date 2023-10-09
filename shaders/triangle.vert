#version 450

layout(location = 0) in vec3 position;
layout(location = 2) in vec4 color;
layout(set = 0, binding = 0) buffer UniformBuffer {
    mat4 projection;
    mat4 view;
} ubo;

layout(location = 0) out vec4 fragColor;

void main() {
	gl_Position = ubo.projection * ubo.view * vec4(position, 1.0f);
	fragColor = color;
}

// vim: ft=glsl
