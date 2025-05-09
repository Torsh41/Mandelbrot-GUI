#version 460

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in float gChunkIndex[];
out vec3 fTexcoord;

layout(std140, binding = 0) uniform shader_data {
    dvec4 window_rec; // values: { x, y, w, h }
    dvec2 chunk_size; // values: { width, height }
};

void main() {
    vec2 ChunkSize = vec2(2 * chunk_size / window_rec.zw);
    gl_Position = gl_in[0].gl_Position;
    fTexcoord = vec3(0.0, 1.0, gChunkIndex[0]);
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + vec4(0.0, ChunkSize.y, 0.0, 0.0);
    fTexcoord = vec3(0.0, 0.0, gChunkIndex[0]);
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + vec4(ChunkSize.x, 0.0, 0.0, 0.0);
    fTexcoord = vec3(1.0, 1.0, gChunkIndex[0]);
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + vec4(ChunkSize, 0.0, 0.0);
    fTexcoord = vec3(1.0, 0.0, gChunkIndex[0]);
    EmitVertex();
    EndPrimitive();
}

