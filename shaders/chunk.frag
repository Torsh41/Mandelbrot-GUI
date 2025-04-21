#version 460

in vec3 fTexcoord;
out vec4 outColor;
uniform sampler2DArray chunk_array;

void main() {
    outColor = texture(chunk_array, fTexcoord).rrrr;
}

