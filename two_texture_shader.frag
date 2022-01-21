#version 330 core

in vec2 texture_coord;
in vec3 normal;
out vec4 frag_color;

uniform sampler2D texture0;
uniform sampler2D texture1;

void main() {
  frag_color = mix(texture(texture0, texture_coord), texture(texture1,
    texture_coord), 0.2);
}

