#version 460

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in float gChunkIndex[];
out vec3 fTexcoord;

void main() {
    const float offset = 0.3;
    gl_Position = gl_in[0].gl_Position + vec4(-offset, -offset, 0.0, 0.0);
    fTexcoord = vec3(0.0, 1.0, gChunkIndex[0]);
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + vec4(-offset, offset, 0.0, 0.0);
    fTexcoord = vec3(0.0, 0.0, gChunkIndex[0]);
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + vec4(offset, -offset, 0.0, 0.0);
    fTexcoord = vec3(1.0, 1.0, gChunkIndex[0]);
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + vec4(offset, offset, 0.0, 0.0);
    fTexcoord = vec3(1.0, 0.0, gChunkIndex[0]);
    EmitVertex();
    EndPrimitive();
}

