#version 460

in vec2 position;
in float chunk_index;
out float gChunkIndex;

layout(std140, binding = 0) uniform shader_data {
    vec4 window_rec; // values: { x, y, w, h }
    vec2 chunk_size; // values: { width, height }
};

void main() {
    gChunkIndex = chunk_index;
    // normalize position to (-1, 1), relative to window position
    vec2 p = 2 * (position.xy - window_rec.xy) / window_rec.zw - 1;
    gl_Position = vec4(p, 0.0, 1.0);
}
