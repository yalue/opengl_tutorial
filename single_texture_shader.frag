#version 330 core

in vec2 texture_coord;
in vec3 normal;
out vec4 frag_color;

uniform sampler2D texture0;

void main() {
  frag_color = texture(texture0, texture_coord);
}
