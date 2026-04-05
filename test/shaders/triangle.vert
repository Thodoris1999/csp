#version 450

// Push constant block: mat4 transform (offset=0, size=64 bytes)
layout(push_constant) uniform PushConstants {
    mat4 transform;
} pc;

// Vertex input
layout(location = 0) in vec3 position;

// Output to fragment shader
layout(location = 0) out vec2 uv;

void main() {
    gl_Position = pc.transform * vec4(position, 1.0);
    // Generate simple UV coordinates from clip-space position
    uv = (gl_Position.xy / gl_Position.w) * 0.5 + 0.5;
}
