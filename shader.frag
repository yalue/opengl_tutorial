#version 330 core

in vec3 vertex_color;
in vec2 texture_coord;
out vec4 frag_color;

uniform sampler2D texture_sampler;

void main() {
  frag_color = texture(texture_sampler, texture_coord) *
    vec4(vertex_color, 1.0);
}

