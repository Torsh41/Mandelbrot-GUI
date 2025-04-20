#version 460

in vec2 position;
in float chunk_index;
out float gChunkIndex;

void main() {
    gChunkIndex = chunk_index;
    gl_Position = vec4(position, 0.0, 1.0);
}
