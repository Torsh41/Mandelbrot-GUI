#version 460

in dvec2 position;
in double chunk_index;
out float gChunkIndex;

layout(std140, binding = 0) uniform shader_data {
    dvec4 window_rec; // values: { x, y, w, h }
    dvec2 chunk_size; // values: { width, height }
};

void main() {
    gChunkIndex = float(chunk_index);
    // gChunkIndex = 0.0f;
    // normalize position to (-1, 1), relative to window position
    vec2 p = vec2(2 * (position.xy - window_rec.xy) / window_rec.zw - 1);
    // hide chunks that are uninitialized
    if (chunk_index == 0.0f) {
        gl_Position = vec4(p, -100.0, 1.0);
    } else {
        gl_Position = vec4(p, 0.0, 1.0);
    }
}
