#version 450

layout(set = 0, binding = 0) buffer MVP {
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

layout(location = 0) in vec3 position;
layout(location = 2) in vec4 color;

layout(location = 0) out vec4 fragColor;

void main() {
	gl_Position = vec4(position, 1.0f);
	fragColor = color;
}

// vim: ft=glsl
