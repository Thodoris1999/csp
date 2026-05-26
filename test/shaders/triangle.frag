#version 450

// Combined image sampler (reflected as CombinedImageSampler)
layout(set = 0, binding = 0) uniform sampler2D tex;

// Uniform buffer (reflected as UniformBuffer)
layout(set = 0, binding = 1) uniform MaterialData {
    vec4 tint;
} material;

// Input from vertex shader
layout(location = 0) in vec2 uv;

// Fragment output
layout(location = 0) out vec4 color;

void main() {
    color = texture(tex, uv) * material.tint;
}
