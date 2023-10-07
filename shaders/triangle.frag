#version 450

layout(set = 0, binding = 0) buffer FragmentData {
    vec4 color[3];
};

//output write
layout (location = 0) out vec4 outFragColor;

void main() {
	outFragColor = vec4(1.f, 0.f, 0.f, 1.0f);
}

// vim: ft=glsl
