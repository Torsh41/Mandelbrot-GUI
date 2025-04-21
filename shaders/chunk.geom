#version 460

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in float gChunkIndex[];
out vec3 fTexcoord;

layout(std140, binding = 0) uniform shader_data {
    vec4 window_rec; // values: { x, y, w, h }
};

void main() {
    vec2 chunk_size = 2 * vec2(0.1, 0.1) / window_rec.zw;
    gl_Position = gl_in[0].gl_Position + vec4(0.0, 0.0, 0.0, 0.0);
    fTexcoord = vec3(0.0, 1.0, gChunkIndex[0]);
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + vec4(0, chunk_size.y, 0.0, 0.0);
    fTexcoord = vec3(0.0, 0.0, gChunkIndex[0]);
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + vec4(chunk_size.x, 0, 0.0, 0.0);
    fTexcoord = vec3(1.0, 1.0, gChunkIndex[0]);
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + vec4(chunk_size, 0.0, 0.0);
    fTexcoord = vec3(1.0, 0.0, gChunkIndex[0]);
    EmitVertex();
    EndPrimitive();
}

