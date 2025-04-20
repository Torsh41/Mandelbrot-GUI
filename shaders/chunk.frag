#version 460

in vec3 fTexcoord;
out vec4 outColor;
uniform sampler2DArray chunk;
void main() {
    outColor = texture(chunk, fTexcoord).rrrr;
}

