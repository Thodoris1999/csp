#version 450

// Uniform sampler for texture lookup
layout(binding = 0) uniform sampler2D tex;

// Input from vertex shader
layout(location = 0) in vec2 uv;

// Fragment output
layout(location = 0) out vec4 color;

void main() {
    color = texture(tex, uv);
}
